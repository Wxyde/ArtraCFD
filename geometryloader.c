/****************************************************************************
 * Geometry Data Loader                                                     *
 * Programmer: Huangrui Mo                                                  *
 * - Follow the Google's C/C++ style Guide.                                 *
 * - This file defines a loader for geometry data                           *
 ****************************************************************************/
/****************************************************************************
 * Required Header Files
 ****************************************************************************/
#include "geometryloader.h"
#include <stdio.h> /* standard library for input and output */
#include <string.h> /* manipulating strings */
#include "commons.h"
/****************************************************************************
 * Static Function Declarations
 ****************************************************************************/
static int NonrestartGeometryLoader(Particle *);
static int RestartGeometryLoader(Particle *);
static int ReadGeometryData(FILE **, Particle *);
/****************************************************************************
 * Function definitions
 ****************************************************************************/
/*
 * This function load geometry data from the geometry file.
 */
int LoadGeometryData(Particle *particle, const Time *time)
{
    if (0 == time->restart) { /* if non-restart, read input file */
        NonrestartGeometryLoader(particle);
    } else { /* if restart, read the particle file */
        RestartGeometryLoader(particle);
    }
    return 0;
}
static int NonrestartGeometryLoader(Particle *particle)
{
    ShowInformation("Loading inputed geometry data ...");
    FILE *filePointer = fopen("artracfd.geo", "r");
    if (NULL == filePointer) {
        FatalError("failed to open geometry file: artracfd.geo...");
    }
    /* read and process file line by line */
    char currentLine[200] = {'\0'}; /* store the current read line */
    int entryCount = 0; /* entry count */
    while (NULL != fgets(currentLine, sizeof currentLine, filePointer)) {
        CommandLineProcessor(currentLine); /* process current line */
        if (0 == strncmp(currentLine, "sphere count begin", sizeof currentLine)) {
            ++entryCount;
            fgets(currentLine, sizeof currentLine, filePointer);
            sscanf(currentLine, "%d", &(particle->totalN)); 
            continue;
        }
        if (0 == strncmp(currentLine, "sphere begin", sizeof currentLine)) {
            ++entryCount;
            ReadGeometryData(&filePointer, particle);
        }
        continue;
    }
    fclose(filePointer); /* close current opened file */
    /* Check missing information section in configuration */
    if (2 != entryCount) {
        FatalError("missing or repeated necessary information section");
    }
    ShowInformation("Session End");
    return 0;
}
static int RestartGeometryLoader(Particle *particle)
{
    ShowInformation("Restore geometry data ...");
    /* restore geometry information from particle file. */
    FILE *filePointer = fopen("restart.particle", "r");
    if (NULL == filePointer) {
        FatalError("failed to open restart particle file: restart.particle...");
    }
    /* read and process file line by line */
    char currentLine[200] = {'\0'}; /* store the current read line */
    fgets(currentLine, sizeof currentLine, filePointer);
    sscanf(currentLine, "N: %d", &(particle->totalN)); 
    ReadGeometryData(&filePointer, particle);
    fclose(filePointer); /* close current opened file */
    ShowInformation("Session End");
    return 0;
}
static int ReadGeometryData(FILE **filePointerPointer, Particle *particle)
{
    FILE *filePointer = *filePointerPointer; /* get the value of file pointer */
    if (0 == particle->totalN) { /* no internal geometries */
        return 0;
    }
    /* 
     * Assign storage to store particle information:
     * x, y, z, r, density, u, v, w, fx, fy, fz
     */
    particle->entryN = 11;
    particle->headAddress = AssignStorage(particle->totalN * particle->entryN, "Real");
    /* read and store data per object*/
    char currentLine[500] = {'\0'}; /* store the current read line */
    /* set format specifier according to the type of Real */
    char formatXI[100] = "%lg, %lg, %lg, %lg, %lg, %lg, %lg, %lg, %lg, %lg, %lg"; /* default is double type */
    if (sizeof(Real) == sizeof(float)) { /* if set Real as float */
        strncpy(formatXI, "%g, %g, %g, %g, %g, %g, %g, %g, %g, %g, %g", sizeof formatXI); /* float type */
    }
    Real *ptk = NULL;
    for (int geoCount = 0; geoCount < particle->totalN; ++geoCount) {
        ptk = particle->headAddress + geoCount * particle->entryN; /* point to storage of current particle */
        fgets(currentLine, sizeof currentLine, filePointer);
        sscanf(currentLine, formatXI, ptk + 0, ptk + 1, ptk + 2, ptk + 3, ptk + 4, ptk + 5,
                ptk + 6, ptk + 7, ptk + 8, ptk + 9, ptk + 10);
    }
    *filePointerPointer = filePointer; /* return a updated value of file pointer */
    return 0;
}
/* a good practice: end file with a newline */

