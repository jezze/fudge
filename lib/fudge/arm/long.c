__extension__ typedef unsigned long long ulonglong;

struct division
{

    ulonglong quotient;
    ulonglong remainder;

};

/* the long division __aeabi_uldivmod and __aeabi_ldivmod hand their result back from; dividing by 0 gives 0 */
void __udivmoddi4s(struct division *division, ulonglong n, ulonglong d)
{

    unsigned int i;

    division->quotient = 0;
    division->remainder = 0;

    if (!d)
        return;

    for (i = 0; i < 64; i++)
    {

        division->remainder = (division->remainder << 1) | (n >> 63);
        n <<= 1;
        division->quotient <<= 1;

        if (division->remainder >= d)
        {

            division->remainder -= d;
            division->quotient |= 1;

        }

    }

}

/* the quotient truncated toward zero, the remainder taking the dividend's sign */
void __divmoddi4s(struct division *division, ulonglong n, ulonglong d)
{

    unsigned int negativen = n >> 63;
    unsigned int negatived = d >> 63;

    __udivmoddi4s(division, (negativen) ? -n : n, (negatived) ? -d : d);

    if (negativen != negatived)
        division->quotient = -division->quotient;

    if (negativen)
        division->remainder = -division->remainder;

}

