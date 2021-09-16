/*
**  types: int & long double
*/

#include <iostream>
#include <cstdlib>
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
    unsigned char bytes[1];
};

union Integer
{
    int value;
    unsigned char bytes[1];
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
    return *(charBuf + numByte) & mask;
}

//! not-memory-safe function
inline void setBit(void* buf, size_t pos, bool val)
{
    unsigned char* charBuf = (unsigned char*)buf;
    size_t numByte = pos / numBitsInByte;
    size_t numBit = pos % numBitsInByte;

    if (val == true)
    {
        unsigned char mask = 1 << numBit;
        *(charBuf + numByte) = *(charBuf + numByte) | mask;
    }
    else
    {
        unsigned char mask = 0xff - (1 << numBit);
        *(charBuf + numByte) = *(charBuf + numByte) & mask;
    }
}

// param: union(LongDouble or Integer)
template < typename T >
T getSwappedNumber(T number, size_t firstGroupFirstElem, size_t sizeOfFirstGroup,
                                size_t secondGroupFirstElem, size_t sizeOfSecondGroup)
{
    // check input parameters
    if (true)
    {
        // check of non-negativity of group first element
        if (firstGroupFirstElem < 0 || secondGroupFirstElem < 0)
            throw std::logic_error("first index of group < 0");

        // check of positivity of group size
        if (sizeOfFirstGroup <= 0 || sizeOfSecondGroup <= 0)
            throw std::logic_error("size of group <= 0");

        //! что делать с sizeof(__float80)???
        // check do groups fit into type size
        if (firstGroupFirstElem + sizeOfFirstGroup >= sizeof(number.value) * numBitsInByte)
            throw std::logic_error("first group does not fit into type size");

        if (secondGroupFirstElem + sizeOfSecondGroup >= sizeof(number.value) * numBitsInByte)
            throw std::logic_error("second group does not fit into type size");

        // check do groups intersect
        size_t firstGroupLastElem = firstGroupFirstElem + sizeOfFirstGroup - 1;
        size_t secondGroupLastElem = secondGroupFirstElem + sizeOfSecondGroup - 1;
        if (firstGroupLastElem >= secondGroupFirstElem && firstGroupLastElem <= secondGroupLastElem ||
            secondGroupLastElem >= firstGroupFirstElem && secondGroupLastElem <= firstGroupLastElem)
            throw std::logic_error("groups intersect");
    }

    // move the gap begween groups
    Т copyOfNumber = number;
    size_t firstGapElem = firstGroupFirstElem + sizeOfFirstGroup;
    size_t sizeOfGap = secondGroupFirstElem - firstGapElem;
    // gap offset may be positive or negative
    int gapOffset = (int)(firstGroupFirstElem + sizeOfFirstGroup) - 
                    (int)(secondGroupFirstElem + sizeOfSecondGroup);
    for (i = 0; i < sizeOfGap; i++)
    {
        bool bit = getBit(number.bits, firstGapElem + i);
        setBit(copyOfNumber.bits, i + gapOffset);
    }
    // copy bits from first group
    for (i = 0; i < sizeOfFirstGroup; i++)
    {
        bool bit = getBit(number.bits, firstGroupFirstElem + i);
        setBit(copyOfNumber.bits, secondGroupFirstElem, bit)
    }
    // copy bits from second group
    for (i = 0; i < sizeOfSecondGroup; i++)
    {
        bool bit = getBit(number.bits, secondGroupFirstElem + i);
        setBit(copyOfNumber.bits, firstGroupFirstElem, bit)
    }
    return copyOfNumber;
}


int main()
{
    // unsigned int a = 0x04000446;
    // printBinaryValue(a);
    // std::cout << "\n";
    // printPlaces(sizeof(a) * numBitsInByte);
    // std::cout << "\n";
    //
    // setBit(&a, 10, false);
    // setBit(&a, 2, false);
    // setBit(&a, 26, false);
    // setBit(&a, 31, true);
    // setBit(&a, 28, true);
    //
    // printBinaryValue(a);
    // std::cout << "\n";
    // printPlaces(sizeof(a) * numBitsInByte);
    // return 0;

    Integer a = {0x04000446, nullptr};
    printBinaryValue(getSwappedNumber(a, 1, 5, 9, 6).value);
    std::cout << "\n";
    printPlaces(sizeof(a) * numBitsInByte);
    std::cout << "\n";

    // union A
    // {
    //     char a[4];
    //     short b[2];
    // };
    // A u;
    // u.b[0] = 256;
    // u.b[1] = 512;
    // std::cout << (int)u.a[0] << (int)u.a[1] << (int)u.a[2] << (int)u.a[3];
    // return 0;

    for (;;)
    {
        // состояние - выбор типа числа: i или r
        std::cout << "Enter number type: i (integer) or r (real)\n>";
        char numberType;
        std::cin >> numberType;
        if (numberType != 'i' && numberType != 'r')
        {
            std::cout << "Error: unknown type of number";
            continue;
        }

        bool isInteger = numberType == 'i';
        if (isInteger)
        {
            for (;;)
            {
                // состояние - ввод числа
                std::cout << "Enter integer number\ni>";
                std::string strN;
                std::cin >> strN;
                if (strN == "..")
                    break;

                int n;
                try
                {
                    n = std::stoi(strN);
                    std::cout << n << "\n";
                    printBinaryValue(n);
                    std::cout << "\n";
                    printPlaces(sizeof(n) * numBitsInByte);
                }
                catch(std::logic_error& e)
                {
                    std::cerr << "Error: incorrect number\n";
                }
            }
        }
        else
        {
            for (;;)
            {
                // состояние - ввод числа
                std::cout << "Enter real number\nr>";
                std::string strN;
                std::cin >> strN;
                if (strN == "..")
                        break;

                LongDouble n;
                try
                {
                    n.value = std::stold(strN);
                    std::cout << n.value << "\n";
                    printBinaryValue(n.Parts.p);
                    printBinaryValue(n.Parts.m);
                    std::cout << "\n";
                    printPlaces(sizeof(n.Parts) * numBitsInByte);
                }
                catch(std::logic_error& e)
                {
                    std::cerr << "Error: incorrect number\n";
                }
            }
        }
    }
}