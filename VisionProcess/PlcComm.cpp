#include "PlcComm.h"

char PlcComm::GetPlcFd()
{
	/*
        Get sensor state form PLC EB struct.
        If bit == 1 -> There is a box in the system.
        If bit == 0 -> There is no box.
        Returns: char fd state.
        */

        // Struct with the data information (PLC EB byte).
        byte EB[16]; // 16 Digital Input bytes.
        TS7DataItem Items;
        Items.Area = S7AreaPE;
        Items.WordLen = S7WLByte;
        Items.DBNumber = 0;
        Items.Start = 21;
        Items.Amount = 1;
        Items.pdata = &EB;

        int res = MyClient->ReadMultiVars(&Items, 1);

        if (Items.Result == 0)
        {
            // Hex value of the byte.
            char hexVal = 0xFF & ((char*)&EB)[0];

            // Transform hex byte to binary.
            bitset<16> binVal(hexVal);
            string binStr = binVal.to_string();
            char binArray[16] = { 0 };
            copy(binStr.begin(), binStr.end(), binArray);

            // Return the 12th position of the byte.
            char fd = binArray[12];
            return fd;
        }
}

int PlcComm::GetPlcNum()
{
    /*
    Get the box code from PLC MB (integer number).
    Returns: int boxCode
    */
    int mbIni = 84;
    const int len = 2;
    int boxCode;
    byte mbValue[len] = { 0 };

    // Lee la MW94 y MW96.
    MyClient->MBRead(mbIni, len, &mbValue);

    // Convierte los valores de lectura en enteros.
    boxCode = int(0xFF & ((char*)mbValue)[1]);

    return boxCode;
}
