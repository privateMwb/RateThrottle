#!/usr/bin/env python3

import argparse
import hashlib
import json
import pathlib
import re
import sys


def parse_submodule(value: str) -> tuple[str, str, str, str]:
    # NAME=repo:sha:archive
    try:
        name, rest = value.split("=", 1)
        sub_repo, sub_sha, sub_archive = rest.split(":", 2)
    except ValueError:
        sys.exit(
            "Malformed --submodule value (expected NAME=repo:sha:archive): "
            f"{value}"
        )
    return name, sub_repo, sub_sha, sub_archive


parser = argparse.ArgumentParser()

parser.add_argument("--package", required=True)
parser.add_argument("--repo", required=True)
parser.add_argument("--version", required=True)
parser.add_argument("--ref", required=True)
parser.add_argument("--archive", required=True)
parser.add_argument("--root-dir", required=True)
parser.add_argument("--conan-dir", required=True)
parser.add_argument("--vcpkg-dir", required=True)
parser.add_argument(
    "--submodule",
    action="append",
    default=[],
    metavar="NAME=REPO:SHA:ARCHIVE",
    help="Internal git submodule pin, repeatable. Example: "
         "FunctionPro=privateMwb/FunctionPro:1c63b93...:FunctionPro.tar.gz",
)

args = parser.parse_args()

package = args.package
repo = args.repo
version = args.version
ref = args.ref

submodules = [parse_submodule(s) for s in args.submodule]

archive = pathlib.Path(args.archive)

if not archive.exists():
    sys.exit(f"Archive not found: {archive}")

data = archive.read_bytes()

sha256 = hashlib.sha256(data).hexdigest()
sha512 = hashlib.sha512(data).hexdigest()

print(f"Version : {version}")
print(f"Ref     : {ref}")
print(f"SHA256  : {sha256}")
print(f"SHA512  : {sha512}")

# repo -> (ref, sha512), used to rewrite each vcpkg_from_github() block
# below. Keyed by repo rather than submodule name, since that's what
# each block's REPO line carries.
repo_pins = {repo: (ref, sha512)}

for name, sub_repo, sub_sha, sub_archive_path in submodules:
    sub_archive = pathlib.Path(sub_archive_path)

    if not sub_archive.exists():
        sys.exit(f"Submodule archive not found: {sub_archive}")

    sub_data = sub_archive.read_bytes()
    sub_sha512 = hashlib.sha512(sub_data).hexdigest()

    print(f"  Submodule {name}: ref={sub_sha} sha512={sub_sha512}")

    repo_pins[sub_repo] = (sub_sha, sub_sha512)

# ---------------------------------------------------------------------
# Root CMakeLists.txt
# ---------------------------------------------------------------------

root_cmakelists = pathlib.Path(args.root_dir) / "CMakeLists.txt"

text = root_cmakelists.read_text(encoding="utf-8")


def _bump_project_version(match: re.Match) -> str:
    # Substitute only within the matched project(...) call, so this
    # can't touch an unrelated VERSION elsewhere in the file (e.g.
    # cmake_minimum_required(VERSION ...)).
    return re.sub(
        r"VERSION\s+[0-9A-Za-z.\-_]+",
        f"VERSION {version}",
        match.group(0),
        count=1,
    )


new_text, count = re.subn(
    r"project\s*\([^)]*\)",
    _bump_project_version,
    text,
    count=1,
    flags=re.DOTALL,
)

if count == 0:
    sys.exit(f"No project() call found in {root_cmakelists}")

root_cmakelists.write_text(new_text, encoding="utf-8")

print("✓ Updated CMakeLists.txt")

# ---------------------------------------------------------------------
# Conan
# ---------------------------------------------------------------------

recipe_dir = pathlib.Path(args.conan_dir)

# conanfile.py

conanfile = recipe_dir / "conanfile.py"

text = conanfile.read_text(encoding="utf-8")

text = re.sub(
    r'version\s*=\s*"[^"]+"',
    f'version = "{version}"',
    text,
)

conanfile.write_text(text, encoding="utf-8")

print("✓ Updated conanfile.py")

# conandata.yml
#
# conanfile.py's source() clones the repo directly and resolves
# submodules via `git submodule update --init --recursive`, which
# reads each submodule's pin straight from .gitmodules -- so this
# only needs the main repo's clone URL and commit, not a tarball
# sha256 and not any per-submodule data.

conandata = recipe_dir / "conandata.yml"

conandata.write_text(
f'''sources:
  "{version}":
    url: "https://github.com/{repo}.git"
    commit: "{ref}"
''',
encoding="utf-8"
)

print("✓ Updated conandata.yml")

# ---------------------------------------------------------------------
# vcpkg
# ---------------------------------------------------------------------

port_dir = pathlib.Path(args.vcpkg_dir)

# portfile.cmake
#
# GitHub tarballs don't include submodule content, so each internal
# submodule is fetched with its own vcpkg_from_github() block, in
# addition to the one for the main package. Every block is rewritten
# by matching its REPO line against repo_pins, rather than assuming
# block order matches --submodule argument order.

portfile = port_dir / "portfile.cmake"

text = portfile.read_text(encoding="utf-8")

BLOCK_RE = re.compile(
    r"vcpkg_from_github\(\s*"
    r"OUT_SOURCE_PATH\s+\S+\s*"
    r"REPO\s+(\S+)\s*"
    r"REF\s+\S+\s*"
    r"SHA512\s+\S+\s*"
    r"\)"
)


def _rewrite_block(match: re.Match) -> str:
    block_repo = match.group(1)
    pin = repo_pins.get(block_repo)

    if pin is None:
        sys.exit(f"No pin supplied for REPO {block_repo} in portfile.cmake")

    block_ref, block_sha512 = pin
    block_text = match.group(0)
    block_text = re.sub(r"REF\s+\S+", f"REF {block_ref}", block_text, count=1)
    block_text = re.sub(
        r"SHA512\s+\S+", f"SHA512 {block_sha512}", block_text, count=1
    )
    return block_text


new_text, count = BLOCK_RE.subn(_rewrite_block, text)

if count == 0:
    sys.exit(f"No vcpkg_from_github() blocks found in {portfile}")

portfile.write_text(new_text, encoding="utf-8")

print(f"✓ Updated portfile.cmake ({count} vcpkg_from_github block(s))")

# vcpkg.json

vcpkg_json = port_dir / "vcpkg.json"

data = json.loads(vcpkg_json.read_text(encoding="utf-8"))

data["version"] = version

vcpkg_json.write_text(
    json.dumps(data, indent=2) + "\n",
    encoding="utf-8",
)

print("✓ Updated vcpkg.json")