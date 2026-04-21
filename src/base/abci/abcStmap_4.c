/**CFile****************************************************************

  FileName    [abcStmap_4.c]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [Network and node package.]

  Synopsis    [Versioned downstream-aware mapper wrapper.]

***********************************************************************/

#include "base/abc/abc.h"
#include "base/main/main.h"
#include "map/mio/mio.h"
#include "misc/extra/extra.h"

ABC_NAMESPACE_IMPL_START

/**Function*************************************************************

  Synopsis    [Runs stmap4.]

  Description [Iteration 4 keeps stmap1's logarithmic fanout-delay
  estimate and skip-fanout filter enabled, but relaxes the hard
  skip thresholds from (3-leaf > 3 refs, 4-leaf > 1 ref) to
  (3-leaf > 5 refs, 4-leaf > 2 refs). The hypothesis is that the
  smaller designs need more freedom on moderately shared nodes,
  while the larger designs still benefit from filtering only the
  highest-fanout matches.]

  SideEffects []

  SeeAlso     []

***********************************************************************/
int Abc_CommandStmap4( Abc_Frame_t * pAbc, int argc, char ** argv )
{
    Abc_Ntk_t * pNtk, * pNtkRes;
    char Buffer[100];
    double DelayTarget;
    double AreaMulti;
    double DelayMulti;
    float LogFan;
    float Slew;
    float Gain;
    int nGatesMin;
    int fAreaOnly;
    int fRecovery;
    int fSweep;
    int fSwitching;
    int fSkipFanout;
    int fUseProfile;
    int fUseBuffs;
    int fVerbose;
    int c;
    extern Abc_Ntk_t * Abc_NtkMapSelectiveFanout( Abc_Ntk_t * pNtk, Mio_Library_t * userLib, double DelayTarget, double AreaMulti, double DelayMulti, float LogFan, float Slew, float Gain, int nGatesMin, int fRecovery, int fSwitching, int fSkipFanout, int nSkipFanoutRefs3, int nSkipFanoutRefs4, int fUseProfile, int fUseBuffs, int fVerbose );
    extern int Abc_NtkFraigSweep( Abc_Ntk_t * pNtk, int fUseInv, int fExdc, int fVerbose, int fVeryVerbose );

    pNtk = Abc_FrameReadNtk( pAbc );
    DelayTarget = -1;
    AreaMulti   = 0;
    DelayMulti  = 0;
    LogFan      = 1.0;
    Slew        = 0;
    Gain        = 250;
    nGatesMin   = 0;
    fAreaOnly   = 0;
    fRecovery   = 1;
    fSweep      = 0;
    fSwitching  = 0;
    fSkipFanout = 1;
    fUseProfile = 0;
    fUseBuffs   = 1;
    fVerbose    = 0;

    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "DABFSGMarspfuovh" ) ) != EOF )
    {
        switch ( c )
        {
        case 'D':
            if ( globalUtilOptind >= argc )
            {
                Abc_Print( -1, "Command line switch \"-D\" should be followed by a floating point number.\n" );
                goto usage;
            }
            DelayTarget = atof( argv[globalUtilOptind] );
            globalUtilOptind++;
            if ( DelayTarget <= 0.0 )
                goto usage;
            break;
        case 'A':
            if ( globalUtilOptind >= argc )
            {
                Abc_Print( -1, "Command line switch \"-A\" should be followed by a floating point number.\n" );
                goto usage;
            }
            AreaMulti = atof( argv[globalUtilOptind] );
            globalUtilOptind++;
            break;
        case 'B':
            if ( globalUtilOptind >= argc )
            {
                Abc_Print( -1, "Command line switch \"-B\" should be followed by a floating point number.\n" );
                goto usage;
            }
            DelayMulti = atof( argv[globalUtilOptind] );
            globalUtilOptind++;
            break;
        case 'F':
            if ( globalUtilOptind >= argc )
            {
                Abc_Print( -1, "Command line switch \"-F\" should be followed by a floating point number.\n" );
                goto usage;
            }
            LogFan = (float)atof( argv[globalUtilOptind] );
            globalUtilOptind++;
            if ( LogFan < 0.0 )
                goto usage;
            break;
        case 'S':
            if ( globalUtilOptind >= argc )
            {
                Abc_Print( -1, "Command line switch \"-S\" should be followed by a floating point number.\n" );
                goto usage;
            }
            Slew = (float)atof( argv[globalUtilOptind] );
            globalUtilOptind++;
            if ( Slew <= 0.0 )
                goto usage;
            break;
        case 'G':
            if ( globalUtilOptind >= argc )
            {
                Abc_Print( -1, "Command line switch \"-G\" should be followed by a floating point number.\n" );
                goto usage;
            }
            Gain = (float)atof( argv[globalUtilOptind] );
            globalUtilOptind++;
            if ( Gain <= 0.0 )
                goto usage;
            break;
        case 'M':
            if ( globalUtilOptind >= argc )
            {
                Abc_Print( -1, "Command line switch \"-M\" should be followed by a positive integer.\n" );
                goto usage;
            }
            nGatesMin = atoi( argv[globalUtilOptind] );
            globalUtilOptind++;
            if ( nGatesMin < 0 )
                goto usage;
            break;
        case 'a':
            fAreaOnly ^= 1;
            break;
        case 'r':
            fRecovery ^= 1;
            break;
        case 's':
            fSweep ^= 1;
            break;
        case 'p':
            fSwitching ^= 1;
            break;
        case 'f':
            fSkipFanout ^= 1;
            break;
        case 'u':
            fUseProfile ^= 1;
            break;
        case 'o':
            fUseBuffs ^= 1;
            break;
        case 'v':
            fVerbose ^= 1;
            break;
        case 'h':
            goto usage;
        default:
            goto usage;
        }
    }

    if ( pNtk == NULL )
    {
        Abc_Print( -1, "Empty network.\n" );
        return 1;
    }

    if ( fAreaOnly )
        DelayTarget = ABC_INFINITY;

    if ( !Abc_NtkIsStrash( pNtk ) )
    {
        pNtk = Abc_NtkStrash( pNtk, 0, 0, 0 );
        if ( pNtk == NULL )
        {
            Abc_Print( -1, "Strashing before mapping has failed.\n" );
            return 1;
        }
        pNtk = Abc_NtkBalance( pNtkRes = pNtk, 0, 0, 1 );
        Abc_NtkDelete( pNtkRes );
        if ( pNtk == NULL )
        {
            Abc_Print( -1, "Balancing before mapping has failed.\n" );
            return 1;
        }
        Abc_Print( 0, "The network was strashed and balanced before stmap4.\n" );
        pNtkRes = Abc_NtkMapSelectiveFanout( pNtk, NULL, DelayTarget, AreaMulti, DelayMulti, LogFan, Slew, Gain, nGatesMin, fRecovery, fSwitching, fSkipFanout, 5, 2, fUseProfile, fUseBuffs, fVerbose );
        if ( pNtkRes == NULL )
        {
            Abc_NtkDelete( pNtk );
            Abc_Print( -1, "stmap4 has failed.\n" );
            return 1;
        }
        Abc_NtkDelete( pNtk );
    }
    else
    {
        pNtkRes = Abc_NtkMapSelectiveFanout( pNtk, NULL, DelayTarget, AreaMulti, DelayMulti, LogFan, Slew, Gain, nGatesMin, fRecovery, fSwitching, fSkipFanout, 5, 2, fUseProfile, fUseBuffs, fVerbose );
        if ( pNtkRes == NULL )
        {
            Abc_Print( -1, "stmap4 has failed.\n" );
            return 1;
        }
    }

    if ( fSweep )
    {
        Abc_NtkFraigSweep( pNtkRes, 0, 0, 0, 0 );
        if ( Abc_NtkHasMapping( pNtkRes ) )
        {
            pNtkRes = Abc_NtkDupDfs( pNtk = pNtkRes );
            Abc_NtkDelete( pNtk );
        }
    }

    Abc_FrameReplaceCurrentNetwork( pAbc, pNtkRes );
    return 0;

usage:
    if ( DelayTarget == -1 )
        snprintf( Buffer, sizeof(Buffer), "not used" );
    else
        snprintf( Buffer, sizeof(Buffer), "%.3f", DelayTarget );
    Abc_Print( -2, "usage: stmap4 [-DABFSG float] [-M num] [-arspfuovh]\n" );
    Abc_Print( -2, "\t              performs version 4 downstream-aware standard cell mapping\n" );
    Abc_Print( -2, "\t              defaults differ from stmap1 by relaxing skip-fanout thresholds to 3-leaf>5 refs and 4-leaf>2 refs\n" );
    Abc_Print( -2, "\t-D float : sets the global required times [default = %s]\n", Buffer );
    Abc_Print( -2, "\t-A float : \"area multiplier\" to bias gate selection [default = %.2f]\n", AreaMulti );
    Abc_Print( -2, "\t-B float : \"delay multiplier\" to bias gate selection [default = %.2f]\n", DelayMulti );
    Abc_Print( -2, "\t-F float : the logarithmic fanout delay parameter [default = %.2f]\n", LogFan );
    Abc_Print( -2, "\t-S float : the slew parameter used to generate the library [default = %.2f]\n", Slew );
    Abc_Print( -2, "\t-G float : the gain parameter used to generate the library [default = %.2f]\n", Gain );
    Abc_Print( -2, "\t-M num   : skip gate classes whose size is less than this [default = %d]\n", nGatesMin );
    Abc_Print( -2, "\t-a       : toggles area-only mapping [default = %s]\n", fAreaOnly ? "yes" : "no" );
    Abc_Print( -2, "\t-r       : toggles area recovery [default = %s]\n", fRecovery ? "yes" : "no" );
    Abc_Print( -2, "\t-s       : toggles sweep after mapping [default = %s]\n", fSweep ? "yes" : "no" );
    Abc_Print( -2, "\t-p       : optimizes power by minimizing switching [default = %s]\n", fSwitching ? "yes" : "no" );
    Abc_Print( -2, "\t-f       : toggles skipping large gates on high-fanout nodes [default = %s]\n", fSkipFanout ? "yes" : "no" );
    Abc_Print( -2, "\t-u       : toggles use of cell profile [default = %s]\n", fUseProfile ? "yes" : "no" );
    Abc_Print( -2, "\t-o       : toggles use of buffers in the result [default = %s]\n", fUseBuffs ? "yes" : "no" );
    Abc_Print( -2, "\t-v       : toggles verbose printout [default = %s]\n", fVerbose ? "yes" : "no" );
    Abc_Print( -2, "\t-h       : prints the command usage\n");
    return 1;
}

ABC_NAMESPACE_IMPL_END
