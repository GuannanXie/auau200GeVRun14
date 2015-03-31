#ifndef CUTS_H
#define CUTS_H

/* **************************************************
 *  Cuts namespace.
 *
 *  Authors:  Xin Dong        (xdong@lbl.gov)
 *            Michael Lomnitz (mrlomnitz@lbl.gov)
 *            Mustafa Mustafa (mmustafa@lbl.gov)
 *            Jochen Thaeder  (jmthader@lbl.gov)   
 *
 * **************************************************
 */

#include <string>

namespace cuts
{
   extern std::string prescalesFilesDirectoryName;
   //event
   extern UShort_t const triggerWord;
   extern float const vz;
   extern float const vzVpdVz;

   //tracking
   extern int const nHitsFit;
   extern bool const requireHFT;

   //pions
   extern float const nSigmaPion;

   //kaons
   extern float const nSigmaKaon;

   // kaonPion pair cuts
   extern float const cosTheta;
   extern float const dcaDaughters;
   extern float const decayLength;
   extern float const minMass;
   extern float const maxMass;
}
#endif
