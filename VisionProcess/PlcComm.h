#pragma once
#ifndef PLCCOMM_H
# define PLCCOMM_H

#include "snap7.h"

#include <stdio.h>
#include <string>
#include <bitset>

using namespace std;

class PlcComm
{
	/*
    Class for PLC communication.
    Using library snap7.
    */
public:
    TS7Client* MyClient;
    int contCaja = 0; // Contador de lecturas de caja que ha realizado el sistema (nos quedaremos con la 1ª)
    int contImg = 0; // Contador de imagenes adquiridas.

private:
    const char* plcIp = "172.16.47.12";
    int plcRack = 0;
    int plcSlot = 2;

public:
    // Constructor.
    PlcComm()
    {
        // Constructor -> Initialize communication.
        MyClient = new TS7Client();
        MyClient->ConnectTo(plcIp, plcRack, plcSlot);
    }
    // Methods.
    char GetPlcFd();
    int GetPlcNum();
};

#endif