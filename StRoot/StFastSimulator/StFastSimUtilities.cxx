#include <iostream>
#include "TH1D.h"
#include "TF1.h"
#include "TFile.h"
#include "TRandom3.h"

#include "StFastSimConstants.h"
#include "StFastSimUtilities.h"

using namespace FastSimUtilitiesConstants;

StFastSimUtilities::StFastSimUtilities(): mh1Vz(), mf1KaonMomResolution(NULL), mf1PionMomResolution(NULL),
   mh1HftRatio(), mh1DcaZ(), mh1DcaXY(), mh1HftRatio1(), mh1DcaZ1(), mh1DcaXY1()
{
   gRandom->SetSeed();

   // Note that this destructor can fail if any of the input files do not exist.
   // RAII

   TFile f(momResolutionFileName.c_str());
   mf1PionMomResolution = (TF1*)f.Get("fPion")->Clone("fPionMomResolution");
   mf1KaonMomResolution = (TF1*)f.Get("fKaon")->Clone("fKaonMomResolution");
   f.Close();

   TFile fHftRatio(hftRatioFileName.c_str());
   TFile fDca(dcaFileName.c_str());
   TFile fVertex(vertexFileName.c_str());

   for (int ii = 0; ii < nCent; ++ii)
   {
      mh1HftRatio[ii] = (TH1D*)(fHftRatio.Get(Form("mh1HFTRatio1_%i", ii))->Clone(Form("mh1HFTRatio1_%i", ii)));
      mh1Vz[ii]      = (TH1D*)(fVertex.Get(Form("mh1Vz_%i", ii))->Clone(Form("mh1Vz_%i", ii)));

      for (int jj = 0; jj < nPtBins; ++jj)
      {
         mh1DcaXY[ii][jj] = (TH1D*)((fDca.Get(Form("mh3DcaXy_Cent%i_Pt%i", ii, jj)))->Clone(Form("mh3DcaXy_Cent%i_Pt%i", ii, jj)));
         mh1DcaZ[ii][jj]  = (TH1D*)((fDca.Get(Form("mh3DcaZ_Cent%i_Pt%i", ii, jj)))->Clone(Form("mh3DcaZ_Cent%i_Pt%i", ii, jj)));
      }
   }

   fHftRatio.Close();
   fDca.Close();
   fVertex.Close();

   TFile fHftRatio1(hftRatioFileName1.c_str());
   TFile fDca1(dcaFileName1.c_str());

   for (int iParticle = 0; iParticle < nParticles; ++iParticle)
   {
      for (int iEta = 0; iEta < nEtas; ++iEta)
      {
         for (int iVz = 0; iVz < nVzs; ++iVz)
         {
            for (int ii = 0; ii < nCent; ++ii)
            {
               mh1HftRatio1[iParticle][iEta][iVz][ii] =
                  (TH1D*)(fHftRatio1.Get(Form("mh1HFT1PtCentPartEtaVzRatio_%i_%i_%i_%i", iParticle, iEta, iVz, ii))->Clone(Form("mh1HFT1PtCentPartEtaVzRatio_%i_%i_%i_%i", iParticle, iEta, iVz, ii)));
               for (int jj = 0; jj < nPtBins; ++jj)
               {
                  mh1DcaXY1[iParticle][iEta][iVz][ii][jj] =
                     (TH1D*)((fDca1.Get(Form("mh1DcaXyPtCentPartEtaVz_%i_%i_%i_%i_%i", iParticle, iEta, iVz, ii, jj)))->Clone(Form("mh1DcaXyPtCentPartEtaVz_%i_%i_%i_%i_%i", iParticle, iEta, iVz, ii, jj)));
                  mh1DcaZ1[iParticle][iEta][iVz][ii][jj] =
                     (TH1D*)((fDca1.Get(Form("mh1DcaZPtCentPartEtaVz_%i_%i_%i_%i_%i", iParticle, iEta, iVz, ii, jj)))->Clone(Form("mh1DcaZPtCentPartEtaVz_%i_%i_%i_%i_%i", iParticle, iEta, iVz, ii, jj)));
               }
            }
         }
      }
   }

   fHftRatio1.Close();
   fDca1.Close();
}

StFastSimUtilities::~StFastSimUtilities()
{
   // needs to be filled
}

TVector3 StFastSimUtilities::smearPosData(int const iParticleIndex, double const vz, int const cent, TLorentzVector const& rMom, TVector3 const& pos) const
{
   int const iEtaIndex = getEtaIndex(rMom.PseudoRapidity());
   int const iVzIndex = getVzIndex(vz);
   int const iPtIndex = getPtIndex(rMom.Perp());

   float sigmaPosZ = 0;
   float sigmaPosXY = 0;

   if (mh1DcaZ1[iParticleIndex][iEtaIndex][iVzIndex][cent][iPtIndex]->GetEntries())
      sigmaPosZ = mh1DcaZ1[iParticleIndex][iEtaIndex][iVzIndex][cent][iPtIndex]->GetRandom() * 1e4;

   if (mh1DcaXY1[iParticleIndex][iEtaIndex][iVzIndex][cent][iPtIndex]->GetEntries())
      sigmaPosXY = mh1DcaXY1[iParticleIndex][iEtaIndex][iVzIndex][cent][iPtIndex]->GetRandom() * 1e4;

   TVector3 newPos(pos);
   newPos.SetZ(0);
   TVector3 momPerp(-rMom.Vect().Y(), rMom.Vect().X(), 0.0);
   newPos += momPerp.Unit() * sigmaPosXY;

   return TVector3(newPos.X(), newPos.Y(), pos.Z() + sigmaPosZ);
}

TVector3 StFastSimUtilities::getVertex(int const centrality) const
{
   double rdmVz;
   if (mh1Vz[centrality]->GetEntries() == 0) rdmVz = 0.;
   else rdmVz = mh1Vz[centrality]->GetRandom() * 1e4;
   return TVector3(0., 0., rdmVz);
}

bool StFastSimUtilities::matchHft(int const iParticleIndex, double const vz, int const cent, TLorentzVector const& mom) const
{
   int const iEtaIndex = getEtaIndex(mom.PseudoRapidity());
   int const iVzIndex = getVzIndex(vz);
   // int const iPhiIndex = getPhiIndex(mom.Phi());
   int const bin = mh1HftRatio1[iParticleIndex][iEtaIndex][iVzIndex][cent]->FindBin(mom.Perp());
   return gRandom->Rndm() < mh1HftRatio1[iParticleIndex][iEtaIndex][iVzIndex][cent]->GetBinContent(bin);
}

float StFastSimUtilities::dca(TVector3 const& p, TVector3 const& pos, TVector3 const& vertex) const
{
   TVector3 posDiff = pos - vertex;
   return fabs(p.Cross(posDiff.Cross(p)).Unit().Dot(posDiff));
}

float StFastSimUtilities::dca1To2(TVector3 const& p1, TVector3 const& pos1, TVector3 const& p2, TVector3 const& pos2, TVector3& v0) const
{
   TVector3 posDiff = pos2 - pos1;
   TVector3 pu1 = p1.Unit();
   TVector3 pu2 = p2.Unit();
   double pu1Pu2 = pu1.Dot(pu2);
   double g = posDiff.Dot(pu1);
   double k = posDiff.Dot(pu2);
   double s2 = (k - pu1Pu2 * g) / (pu1Pu2 * pu1Pu2 - 1.);
   double s1 = g + s2 * pu1Pu2;
   TVector3 posDca1 = pos1 + pu1 * s1;
   TVector3 posDca2 = pos2 + pu2 * s2;
   v0 = 0.5 * (posDca1 + posDca2);
   return (posDca1 - posDca2).Mag();
}

TLorentzVector StFastSimUtilities::smearMom(TLorentzVector const& b, TF1 const * const fMomResolution) const
{
   float const pt = b.Perp();
   float const sPt = gRandom->Gaus(pt, pt * fMomResolution->Eval(pt));

   TLorentzVector sMom;
   sMom.SetXYZM(sPt * cos(b.Phi()), sPt * sin(b.Phi()), sPt * sinh(b.PseudoRapidity()), b.M());
   return sMom;
}

TVector3 StFastSimUtilities::smearPos(TLorentzVector const& mom, TLorentzVector const& rMom, TVector3 const& pos) const
{
   float thetaMCS = 13.6 / mom.Beta() / rMom.P() / 1000 * sqrt(pxlLayer1Thickness / fabs(sin(mom.Theta())));
   float sigmaMCS = thetaMCS * 28000 / fabs(sin(mom.Theta()));
   float sigmaPos = sqrt(pow(sigmaMCS, 2) + pow(sigmaPos0, 2));

   return TVector3(gRandom->Gaus(pos.X(), sigmaPos), gRandom->Gaus(pos.Y(), sigmaPos), gRandom->Gaus(pos.Z(), sigmaPos));
}

int StFastSimUtilities::getPtIndex(double const pT) const
{
   for (int i = 0; i < nPtBins; i++)
   {
      if ((pT >= ptEdge[i]) && (pT < ptEdge[i + 1]))
         return i;
   }
   return nPtBins - 1 ;
}

int StFastSimUtilities::getEtaIndex(double const Eta) const
{
   for (int i = 0; i < nEtas; i++)
   {
      if ((Eta >= EtaEdge[i]) && (Eta < EtaEdge[i + 1]))
         return i;
   }
   return nEtas - 1 ;
}

int StFastSimUtilities::getVzIndex(double const Vz) const
{
   for (int i = 0; i < nVzs; i++)
   {
      if ((Vz >= VzEdge[i]) && (Vz < VzEdge[i + 1]))
         return i;
   }
   return nVzs - 1 ;
}

int StFastSimUtilities::getPhiIndex(double const Phi) const
{
   for (int i = 0; i < nPtBins; i++)
   {
      if ((Phi >= PhiEdge[i]) && (Phi < PhiEdge[i + 1]))
         return i;
   }
   return nPhis - 1 ;
}