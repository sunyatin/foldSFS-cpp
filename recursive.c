#include <iostream>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <iomanip>


/*
Rémi Tournebize & Yves Vigouroux
start: 8 june 2015
large str modif: 9 june 2015 (instead of first computing a matrix of combinatorics (nrow=2^nInd) the algo computes 1dsfs/pop/site recursively)
*/

/*
Only 3 possible genotypes using a BEAGLE format:
NB. Assumes only diploid individuals!
0 = maj/maj
1 = maj/min
2 = min/min
*/

/*
IMPROVEMENTS
- nPop > 2 & nPop ==1
- only biallelic sites ==> DONE
- minInd ==> DONE
- propose to reduce the SFS if high rate of missing data
- speed up the algorithm by removing geno configures if only zeros at min/min across all inds
- include Hardy Weinberg maximization
*/


using namespace std;

int main()
{

    const int nPop = 2;
    int popDiploidSizes[nPop] = { 4, 4 };
    int maxSites = -1; // -1 if you do not want to limit
    const char* beagleFilePath = "C:/Users/Windows/Desktop/THESE IRD/BAM/0.ANGSD/ownANGSD/testData.beagle";
    const char* outSFSPath = "C:/Users/Windows/Desktop/THESE IRD/BAM/0.ANGSD/ownANGSD/noComb/nc_2dsfs.txt";

// Compute number of diploid individuals
    int nAllDiploInd = 0; for ( int i = 0; i < nPop; i++ ) { nAllDiploInd += popDiploidSizes[i]; }

// Initialize the left bound of columns in each population in the BEAGLE file
    int popBounds[nPop]; popBounds[0] = 3;
    for ( int i = 1; i < nPop; i++ ) { popBounds[i] = popBounds[i-1] + popDiploidSizes[i-1] * 3; }

// Open input file (BEAGLE format)
std::ifstream infile(beagleFilePath);
std::string line;
std::string partial;

// lar = array in which each line of the BEAGLE file is imported
double lar[3+nAllDiploInd*3+10]; // we add 10 more elements "by security"

float sfs1d[2][popDiploidSizes[1]*2+1];

float sfs2d[popDiploidSizes[0]*2+1][popDiploidSizes[1]*2+1];
for ( int i=0; i<popDiploidSizes[0]*2+1; i++ ) { for ( int l=0; l<popDiploidSizes[1]*2+1; l++ ) { sfs2d[i][l] = 0; } }

int index = 0;
// iterate through the BEAGLE file
while ( std::getline(infile, line) )
{
    // read input file line-by-line
    std::istringstream iss(line);
    std::string token;
    int w = 0;
    while( std::getline(iss, token, '\t') ) { // assumes a tab delimitation
        double temp = ::atof(token.c_str());
        lar[w] = temp;
        w++;
    }

// how to deal with unequal population size?

    if ( index != 0 ) { // do not read the first line

        // compute 1d-SFS for each population (for the moment only 2 pops)
        int idp = 1;
        for ( int p=0; p<nPop; p++ ) {

            int nip = popDiploidSizes[p];
            int startcol = popBounds[p];
//for (int i=0;i<8;i++) { std::cout << startcol << " "; }; cout << "\n";

            // initialize SFS [old & new]
            float sfs[2][nip*2+1];
            for ( int i=0; i<=1; i++ ) { for ( int l=0; l<nip*2+1; l++ ) { sfs[i][l] = 0; } }

            // populate 1d-SFS with first individual
            for ( int i=0; i<3; i++ ) {
                sfs[0][i] = lar[i+startcol];
            }

            for ( int i=1; i<nip; i++ ) {
                // set new SFS to 0
                for ( int l=0; l<nip*2+1; l++ ) { sfs[1][l] = 0; }
                // for each genotype
                for ( int j=0; j<=2; j++ ) {
                    for ( int k=0; k<3*i; k++ ) {
                        sfs[1][j+k] += sfs[0][k] * lar[3*i+j+startcol];
                    }
                }
                // replace old by new SFS
                for ( int l=0; l<nip*2+1; l++ ) {
                    sfs[0][l] = sfs[1][l];
                    sfs1d[p][l] = sfs[0][l];
                }
            }

        idp++;
        }

        // compute 2d-SFS
        for ( int i=0; i<popDiploidSizes[0]*2+1; i++ ) {
            for ( int j=0; j<popDiploidSizes[1]*2+1; j++ ) {
                if ( i+j < nAllDiploInd ) {
                    // if minor
                    sfs2d[i][j] += sfs1d[0][i]*sfs1d[1][j];
                } else {
                    // if major
                    sfs2d[nAllDiploInd-i][nAllDiploInd-j] += sfs1d[0][i]*sfs1d[1][j];
                }
            }
        }

    }

    index++;
    if ( index % 10000 == 0 ) { cout << "read " << index << " sites\n"; }
    if ( maxSites > 0 && index == maxSites ) { break; }
}

    infile.close();

    // Output the 2d-SFS
    ofstream osfs;
    osfs.open(outSFSPath);
            for (int k = 0; k < 9; k++)
            {
                for (int j = 0; j < 9; j++) {
                        osfs << std::fixed << std::setprecision(8) << sfs2d[k][j] << " ";
                }
                osfs << "\n";
            }
    osfs.close();

    cout << "Done!\n";
    return 0;
}

