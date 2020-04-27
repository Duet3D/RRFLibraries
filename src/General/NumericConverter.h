/*
 * NumericConverter.h
 *
 *  Created on: 26 Apr 2020
 *      Author: David
 */

#ifndef SRC_GENERAL_NUMERICCONVERTER_H_
#define SRC_GENERAL_NUMERICCONVERTER_H_

#include <cstdint>
#include <functional>

// Class to read fixed and floating point numbers
class NumericConverter
{
public:
	NumericConverter() noexcept {}
	bool Accumulate(char c, bool acceptNegative, bool acceptReals, std::function<char() /*noexcept*/> NextChar) noexcept;
	bool FitsInInt32() const noexcept;
	bool FitsInUint32() const noexcept;
	int32_t GetInt32() const noexcept;
	uint32_t GetUint32() const noexcept;
	float GetFloat() const noexcept;
	unsigned int GetDigitsAfterPoint() const noexcept;
	bool IsNegative() const noexcept { return isNegative; }

private:
	uint32_t lvalue;
	int fives;
	int twos;
	bool hadDecimalPoint;
	bool hadExponent;
	bool isNegative;
};

#endif /* SRC_GENERAL_NUMERICCONVERTER_H_ */
