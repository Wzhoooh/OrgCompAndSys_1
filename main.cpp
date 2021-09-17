/*
**  types: int & long double
*/

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <string>
#include <vector>
#include <sstream>
#include <string>

static const size_t numBitsInByte = 8;

#pragma pack(1)
union LongDouble
{
    __float80 value;
    struct
    {
        unsigned long long m;
        unsigned short p;
    } Parts;
    unsigned char bytes[sizeof(Parts)];
};

union Integer
{
    int value;
    unsigned char bytes[sizeof(value)];
};
#pragma pack()

// integer values
template < typename T >
void printBinaryValue(T number, std::ostream& os = std::cout)
{
    size_t numberOfBits = sizeof(number) * numBitsInByte;
    unsigned long long mask = 1ULL << (numberOfBits - 1);
    for (int i = 0; i < numberOfBits; i++)
    {
        if ((number & mask) != 0)
            os << "1";
        else
            os << "0";

        mask >>= 1;
    }
}

void printPlaces(size_t numPlaces, std::ostream& os = std::cout)
{
    for (int i = numPlaces - 1; i >= 0; i--)
        os << i / 10;
    os << "\n";

    for (int i = numPlaces - 1; i >= 0; i--)
        os << i % 10;
    os << "\n";
}


//! not-memory-safe function
inline bool getBit(const void* buf, size_t pos)
{
    unsigned char* charBuf = (unsigned char*)buf;
    size_t numByte = pos / numBitsInByte;
    size_t numBit = pos % numBitsInByte;
    unsigned char mask = 1 << numBit;
    return charBuf[numByte] & mask;
}

//! not-memory-safe function
inline void setBit(void* buf, size_t pos, bool val)
{
    unsigned char* charBuf = (unsigned char*)buf;
    size_t numByte = pos / numBitsInByte;
    size_t numBit = pos % numBitsInByte;
    unsigned char mask = 1 << numBit;
    if (val == true)
        charBuf[numByte] |= mask;
    else
        charBuf[numByte] &= ~mask;
}

// param: union(LongDouble or Integer)
template < typename T >
T getSwappedNumber(T number, size_t firstGroupFirstElem, size_t sizeOfFirstGroup,
                                size_t secondGroupFirstElem, size_t sizeOfSecondGroup)
{
    size_t numBits = sizeof(number.bytes) * numBitsInByte;
    // check input parameters
    if (true)
    {
        // check for too big size
        if (firstGroupFirstElem >= numBits || secondGroupFirstElem >= numBits)
            throw std::logic_error("too big index of first element");

        if (sizeOfFirstGroup >= numBits || sizeOfSecondGroup >= numBits)
            throw std::logic_error("too big size of group");

        // check for non-negativity of group first element
        if (firstGroupFirstElem < 0 || secondGroupFirstElem < 0)
            throw std::logic_error("first index of group < 0");

        // check for positivity of group size
        if (sizeOfFirstGroup <= 0 || sizeOfSecondGroup <= 0)
            throw std::logic_error("size of group <= 0");

        // check do groups fit into type size
        if (firstGroupFirstElem + sizeOfFirstGroup >= numBits)
            throw std::logic_error("first group does not fit into type size");

        if (secondGroupFirstElem + sizeOfSecondGroup >= numBits)
            throw std::logic_error("second group does not fit into type size");

        // check for groups intersection
        size_t firstGroupLastElem = firstGroupFirstElem + sizeOfFirstGroup - 1;
        size_t secondGroupLastElem = secondGroupFirstElem + sizeOfSecondGroup - 1;
        if ((firstGroupLastElem >= secondGroupFirstElem && firstGroupLastElem <= secondGroupLastElem) ||
            (secondGroupLastElem >= firstGroupFirstElem && secondGroupLastElem <= firstGroupLastElem))
            throw std::logic_error("groups intersect");
    }

    // change number of groups: 1st group must be before 2nd group
    if (firstGroupFirstElem > secondGroupFirstElem)
    {
        std::swap(firstGroupFirstElem, secondGroupFirstElem);
        std::swap(sizeOfFirstGroup, sizeOfSecondGroup);
    }

    // move the gap begween groups
    auto copyOfNumber = number;
    size_t firstGapElem = firstGroupFirstElem + sizeOfFirstGroup;
    size_t sizeOfGap = secondGroupFirstElem - firstGapElem;
    // gap offset may be positive or negative
    int gapOffset = (int)sizeOfFirstGroup - (int)sizeOfSecondGroup;
    for (int i = 0; i < sizeOfGap; i++)
    {
        bool bit = getBit(number.bytes, firstGapElem + i);
        setBit(copyOfNumber.bytes, firstGapElem + i - gapOffset, bit);
    }
    // copy bits from first group
    for (int i = 0; i < sizeOfFirstGroup; i++)
    {
        bool bit = getBit(number.bytes, firstGroupFirstElem + i);
        setBit(copyOfNumber.bytes, secondGroupFirstElem + i - gapOffset, bit);
    }
    // copy bits from second group
    for (int i = 0; i < sizeOfSecondGroup; i++)
    {
        bool bit = getBit(number.bytes, secondGroupFirstElem + i);
        setBit(copyOfNumber.bytes, firstGroupFirstElem + i, bit);
    }
    return copyOfNumber;
}

void printInteger(Integer integer, std::ostream& os = std::cout)
{
    os << integer.value << "\n";
    printBinaryValue(integer.value, os);
    os << "\n";
    printPlaces(sizeof(integer.bytes) * numBitsInByte, os);
}

void printReal(LongDouble real, std::ostream& os = std::cout)
{
    os << std::setprecision(25) <<real.value << "\n";
    printBinaryValue(real.Parts.p, os);
    printBinaryValue(real.Parts.m, os);
    os << "\n";
    printPlaces(sizeof(real.bytes) * numBitsInByte, os);
}

int main()
{
    enum
    {
        Zero,
        InputInteger,
        InputReal,
        ThereIsInteger,
        ThereIsReal,
        InputOperationInteger,
        InputOperationReal
    } state;

    Integer integer;
    LongDouble real;

    for(state = Zero;;)
    {
        if (state == Zero)
            std::cout << "Enter number type: i (integer) or r (real)\n>";
        else if (state == InputInteger)
            std::cout << "Enter integer number\ninteger>";
        else if (state == InputReal)
            std::cout << "Enter real number\nreal>";
        else if (state == ThereIsInteger || state == ThereIsReal)
            std::cout << "Enter number type: i (integer) or r (real) or enter operation: s (swap)\n>";
        else if (state == InputOperationInteger || state == InputOperationReal)
            std::cout << "Enter parameters: 2 * [position of first element, size of group]\nswap>";

        std::string userInput;
        std::getline(std::cin, userInput);
        
        if (state == Zero)
        {
            if (userInput == "i")
                state = InputInteger;
            else if (userInput == "r")
                state = InputReal;
            else
            {
                std::cout << "Error: unknown type of number\n";
                continue;
            }
        }
        else if (state == InputInteger)
        {
            try
            {
                integer.value = std::stoi(userInput);
                printInteger(integer);
                state = ThereIsInteger;
            }
            catch(std::logic_error& e)
            {
                std::cerr << "Error: " << e.what() <<"\n";
                continue;
            }
        }
        else if (state == InputReal)
        {
            try
            {
                real.value = std::stold(userInput);
                printReal(real);
                state = ThereIsReal;
            }
            catch(std::logic_error& e)
            {
                std::cerr << "Error: " << e.what() <<"\n";
                continue;
            }
        }
        else if (state == ThereIsInteger || state == ThereIsReal)
        {
            if (userInput == "i")
                state = InputInteger;
            else if (userInput == "r")
                state = InputReal;
            else if (userInput == "s")
                if (state == ThereIsInteger)
                    state = InputOperationInteger;
                else if (state == ThereIsReal)
                    state = InputOperationReal;
            else
            {
                std::cout << "Error: unknown type of number or operation\n";
                continue;
            }
        }
        else if (state == InputOperationInteger || state == InputOperationReal)
        {
            std::stringstream is(userInput);
            std::vector<size_t> numParameters;
            try
            {
                for(std::string param; is >> param;)
                    numParameters.push_back(std::stoi(param));
            }
            catch(const std::logic_error& e)
            {
                std::cerr << "Error: " << e.what() << '\n';
                continue;
            }

            if (numParameters.size() != 4)
            {
                std::cerr << "Error: incorrect number of operation parameters\n";
                continue;
            }

            try
            {
                if (state == InputOperationInteger)
                {
                    printInteger(getSwappedNumber(integer, numParameters[0], numParameters[1],
                                                    numParameters[2], numParameters[3]));
                    state = ThereIsInteger;
                }                                    
                else if (state == InputOperationReal)
                {
                    printReal(getSwappedNumber(real, numParameters[0], numParameters[1],
                                                numParameters[2], numParameters[3]));
                    state = ThereIsReal;
                }
            }
            catch(const std::logic_error& e)
            {
                std::cerr << "Error: " << e.what() << '\n';
                continue;
            }
        }
    }
}