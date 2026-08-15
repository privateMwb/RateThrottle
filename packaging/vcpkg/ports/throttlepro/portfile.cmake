vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO privateMwb/RateThrottle
    REF 63c7c8b240ea2911c80b9448024385cf44c68ca1
    SHA512 32cd6914f8113a05ded010f72b9c159b945b64a34a6ef23623d737fba1cafb20235c001ce2ca63fb5044a28bd750ca091d6a3587d78d956c63a435b634a009d8
)

# GitHub archive tarballs never include submodule content, so
# RateThrottle's internal library under libs/internal/ (LRUCache) is
# fetched separately here, pinned to the exact commit the submodule
# points at, then copied into place.
vcpkg_from_github(
    OUT_SOURCE_PATH LRUCACHE_SOURCE_PATH
    REPO privateMwb/LRUCache
    REF b82b2f00aaafdc205693760ec0e0e191752b95b6
    SHA512 c8d606e2ee9814b6abdfeae00d12ac935e04a06aa5bd79efd04ebb065e54a06e0df95f84ca00e19624d91a4d95c70154ad0a0b0b9caa5713fed513eb7574dfde
)

file(REMOVE_RECURSE "${SOURCE_PATH}/libs/internal/LRUCache")
file(RENAME "${LRUCACHE_SOURCE_PATH}" "${SOURCE_PATH}/libs/internal/LRUCache")

set(VCPKG_PORT_NAME ThrottlePro)

# Consumers only need the library itself, not the tests, benchmarks,
# regression tools, or examples. regression/ also fetches a third-party
# dependency via FetchContent at configure time, which requires network
# access that vcpkg's build sandbox does not allow.
vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DBUILD_TESTS=OFF
        -DBUILD_BENCHMARKS=OFF
        -DBUILD_REGRESSION=OFF
        -DBUILD_EXAMPLES=OFF
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(
    PACKAGE_NAME ${VCPKG_PORT_NAME}
    CONFIG_PATH lib/cmake/${VCPKG_PORT_NAME}
)

# This library is compiled (not header-only), so debug binaries are
# real and must be kept — only the duplicate debug/include headers
# are removed.
file(
    REMOVE_RECURSE
    "${CURRENT_PACKAGES_DIR}/debug/include"
)

vcpkg_install_copyright(
    FILE_LIST "${SOURCE_PATH}/LICENSE"
)
