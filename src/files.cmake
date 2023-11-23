set(HEADERS

    ${PROJECT_HEADER_PATH}/utils/function_traits.hpp
    ${PROJECT_HEADER_PATH}/utils/lockable.hpp
    ${PROJECT_HEADER_PATH}/utils/scope_value.hpp
    ${PROJECT_HEADER_PATH}/utils/utility.hpp
    ${PROJECT_HEADER_PATH}/utils/vector.hpp

    ${PROJECT_HEADER_PATH}/assert.hpp
    ${PROJECT_HEADER_PATH}/pimpl.hpp
    ${PROJECT_HEADER_PATH}/preprocessor.hpp

    ${PROJECT_HEADER_PATH}/meta/meta.hpp
    ${PROJECT_HEADER_PATH}/meta/meta_api.hpp
    ${PROJECT_HEADER_PATH}/meta/library_config.hpp
    ${PROJECT_HEADER_PATH}/meta/thread_pool/thread_pool.hpp
    )

set(PRIVATE_HEADERS
    )

set(SOURCES
    ${PROJECT_SOURCE_PATH}/template.cpp
    ${PROJECT_SOURCE_PATH}/thread_pool/thread_pool.cpp
    )
