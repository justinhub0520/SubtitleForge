#pragma once

#include <string>
#include <optional>

namespace subforge {

struct ApiError {
    std::string code;
    std::string message;
    std::string details;
};

template<typename T>
struct ApiResponse {
    bool success;
    std::optional<T> data;
    std::optional<ApiError> error;
};

} // namespace subforge