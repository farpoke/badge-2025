#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <vector>

namespace ui::qr
{

    struct codeword_t {
        uint8_t value = 0;

        constexpr codeword_t() = default;
        constexpr codeword_t(const codeword_t &) = default;
        constexpr codeword_t &operator=(const codeword_t &) = default;

        constexpr explicit codeword_t(uint8_t v) : value(v) {}

        constexpr codeword_t operator+(const codeword_t &rhs) const { return codeword_t(value ^ rhs.value); }

        constexpr codeword_t operator-(const codeword_t &rhs) const { return codeword_t(value ^ rhs.value); }

        constexpr codeword_t &operator+=(const codeword_t &rhs) {
            value ^= rhs.value;
            return *this;
        }

        constexpr codeword_t &operator-=(const codeword_t &rhs) {
            value ^= rhs.value;
            return *this;
        }
    };

    static_assert(sizeof(codeword_t) == 1);

    codeword_t operator*(const codeword_t &lhs, const codeword_t &rhs);
    codeword_t operator/(const codeword_t &lhs, const codeword_t &rhs);

    struct polynomial_t {
        polynomial_t() = default;
        ~polynomial_t() {
            if (_coefficients)
                delete[] _coefficients;
        }

        polynomial_t(const polynomial_t &copy) {
            if (copy._coefficients == nullptr)
                return;
            _degree = copy._degree;
            _coefficients = new codeword_t[_degree + 1];
            for (int i = 0; i <= _degree; i++)
                _coefficients[i] = copy._coefficients[i];
        }

        polynomial_t &operator=(const polynomial_t &copy) {
            if (_degree != copy._degree) {
                if (_coefficients)
                    delete[] _coefficients;
                _degree = copy._degree;
                _coefficients = new codeword_t[_degree + 1];
            }
            if (_degree >= 0) {
                assert(_coefficients && copy._coefficients);
                for (int i = 0; i <= _degree; i++)
                    _coefficients[i] = copy._coefficients[i];
            }
            return *this;
        }

        polynomial_t(polynomial_t &&move) noexcept {
            _coefficients = move._coefficients;
            _degree = move._degree;
            move._coefficients = nullptr;
            move._degree = -1;
        }

        polynomial_t &operator=(polynomial_t &&move) noexcept {
            if (_coefficients)
                delete[] _coefficients;
            _coefficients = move._coefficients;
            _degree = move._degree;
            move._coefficients = nullptr;
            move._degree = -1;
            return *this;
        }

        template<typename T>
        polynomial_t(const std::initializer_list<T>& init) {
            _degree = init.size() - 1;
            _coefficients = new codeword_t[_degree + 1];
            int i = 0;
            for (const auto& k : init)
                _coefficients[i++].value = k;
        }

        polynomial_t(const std::vector<codeword_t> &coefficients, int shift) {
            _degree = coefficients.size() - 1 + shift;
            _coefficients = new codeword_t[_degree + 1];
            for (int i = 0; i <= _degree; i++) {
                if (i < static_cast<int>(coefficients.size()))
                    _coefficients[i] = coefficients[i];
                else
                    _coefficients[i] = {};
            }
        }

        codeword_t &operator[](int index) {
            assert(index >= 0 && index <= _degree);
            return _coefficients[index];
        }

        codeword_t operator[](int index) const {
            assert(index >= 0 && index <= _degree);
            return _coefficients[index];
        }

        std::vector<codeword_t> as_vector() const {
            if (_coefficients)
                return {_coefficients, _coefficients + _degree + 1};
            else
                return {};
        }

    private:
        codeword_t *_coefficients = nullptr;
        int _degree = -1;

        friend polynomial_t operator*(const polynomial_t &lhs, const codeword_t &rhs);
        friend polynomial_t operator*(const polynomial_t &lhs, const polynomial_t &rhs);
        friend polynomial_t operator%(const polynomial_t &lhs, const polynomial_t &rhs);

        friend polynomial_t construct_generator_polynomial(int degree);
    };

    std::vector<codeword_t> get_ec_codewords(const std::vector<codeword_t> &data, int degree);

} // namespace ui::qr
