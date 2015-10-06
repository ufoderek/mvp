#ifndef _BITS_H_
#define _BITS_H_

#include <stdint.h>
#include <bit_opt_v2.h>

class bits
{
    public:
        bits()
        {
        }

        bits(uint32_t val, uint32_t n)
        {
            set(val, n);
        }

        bits& operator=(const bits& right)
        {
            this->val = right.val;
            this->n = right.n;
            return *this;
        }

        void set(uint32_t val, uint32_t n)
        {
            this->val = val & mask(n - 1, 0);
            this->n = n;
        }

        uint32_t operator()(uint32_t n) const
        {
            return (val & mask(n, n)) >> n;
        }

        uint32_t operator()(uint32_t msb, uint32_t lsb) const
        {
            return (val & mask(msb, lsb)) >> lsb;
        }

        const bool operator()(const uint32_t n)
        {
            return (val & mask(n, n)) >> n;
        }

        const uint32_t operator()(const uint32_t msb, const uint32_t lsb)
        {
            return (val & mask(msb, lsb)) >> lsb;
        }

        bits& operator=(const uint32_t val)
        {
            this->val = val;
            return *this;
        }

        bool match(const uint32_t msb, const uint32_t lsb, const uint32_t pattern)
        {
            return (val & mask(msb, lsb)) == ((pattern << lsb)&mask(msb, lsb));
        }

        bool match(const uint32_t n, const uint32_t bit)
        {
            return (val & mask(n)) == ((bit << n)&mask(n));
        }

        void set_bit(const uint32_t n, const uint32_t bit)
        {
            if(bit)
            {
                val |= mask(n);
            }

            else
            {
                val &= imask(n);
            }
        }

        void set_field(const uint32_t msb, const uint32_t lsb, const uint32_t field)
        {
            val = (val & imask(msb, lsb)) | ((field << lsb)&mask(msb, lsb));
        }

        uint32_t n;
        uint32_t val;
};

class bits64
{
    public:
        bits64()
        {
        }

        bits64(uint64_t val, uint32_t n)
        {
            set(val, n);
        }

        bits64& operator=(const bits64& right)
        {
            this->val = right.val;
            this->n = right.n;
            return *this;
        }

        void set(uint64_t val, uint32_t n)
        {
            this->val = val & mask64(n - 1, 0);
            this->n = n;
        }

        uint32_t operator()(uint32_t n) const
        {
            return (val & mask64(n, n)) >> (uint64_t)n;
        }

        uint64_t operator()(uint32_t msb, uint32_t lsb) const
        {
            return (val & mask64(msb, lsb)) >> (uint64_t)lsb;
        }

        const bool operator()(const uint32_t n)
        {
            return (val & mask64(n, n)) >> (uint64_t)n;
        }

        const uint32_t operator()(const uint32_t msb, const uint32_t lsb)
        {
            return (val & mask64(msb, lsb)) >> (uint64_t)lsb;
        }

        bits64& operator=(const uint64_t val)
        {
            this->val = val;
            return *this;
        }

        bool match(const uint32_t msb, const uint32_t lsb, const uint64_t pattern)
        {
            return (val & mask64(msb, lsb)) == ((pattern << lsb)&mask64(msb, lsb));
        }

        bool match(const uint32_t n, const uint32_t bit)
        {
            return (val & mask64(n)) == ((bit << n)&mask64(n));
        }

        void set_bit(const uint32_t n, const uint32_t bit)
        {
            if(bit)
            {
                val |= mask64(n);
            }

            else
            {
                val &= imask64(n);
            }
        }

        void set_field(const uint32_t msb, const uint32_t lsb, const uint64_t field)
        {
            val = (val & imask64(msb, lsb)) | ((field << lsb)&mask64(msb, lsb));
        }

        uint64_t n;
        uint64_t val;
};

#endif
