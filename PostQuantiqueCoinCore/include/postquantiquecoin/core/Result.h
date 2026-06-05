#pragma once
#include <optional>
#include <string>
#include <utility>

namespace pqc {
template <typename T>
class Result {
public:
    static Result Ok(T value) { return Result(std::move(value), {}); }
    static Result Err(std::string error) { return Result(std::nullopt, std::move(error)); }
    bool IsOk() const { return value_.has_value(); }
    bool IsErr() const { return !value_.has_value(); }
    const T& Value() const { return value_.value(); }
    T& Value() { return value_.value(); }
    const std::string& Error() const { return error_; }
private:
    Result(std::optional<T> value, std::string error) : value_(std::move(value)), error_(std::move(error)) {}
    std::optional<T> value_;
    std::string error_;
};
template <>
class Result<void> {
public:
    static Result Ok() { return Result(true, {}); }
    static Result Err(std::string error) { return Result(false, std::move(error)); }
    bool IsOk() const { return ok_; }
    bool IsErr() const { return !ok_; }
    const std::string& Error() const { return error_; }
private:
    Result(bool ok, std::string error) : ok_(ok), error_(std::move(error)) {}
    bool ok_;
    std::string error_;
};
}
