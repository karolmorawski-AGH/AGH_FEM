#include "GlobalData.h"

GlobalData::GlobalData() {
	this->H = this->W = 1;
	this->nH = this->nW = 2;
	this->nN = this->nH * this->nW;
	this->nE = (this->nH - 1) * (this->nW - 1);
	std::cout << "GlobalData has not been loaded, setting default values" << std::endl;
}

GlobalData::GlobalData(double Hc, double Wc, int nHc, int nWc,
	double mTbeginc, double mTimec, double mdTimec, double mCc,
	double mKc, double mRc, double mTambientc, double mConvc) {
	//TODO Temp?
	if (Hc == 0 || Wc == 0 || nHc == 0 || nWc == 0 || mTimec < 0 || mdTimec < 0) {
		std::cout << "GlobalData loaded with incorrect values, check input" << std::endl;
		exit(-1);
	}
	this->H = Hc;
	this->W = Wc;
	this->nH = nHc;
	this->nW = nWc;
	this->nN = nHc * nWc;
	this->nE = (nHc - 1) * (nWc - 1);
	this->mTbegin = mTbeginc;
	this->mTime = mTimec;
	this->mdTime = mdTimec;
	this->mC = mCc;
	this->mK = mKc;
	this->mR = mRc;
	this->mTambient = mTambientc;
	this->mConv = mConvc;
}

void GlobalData::print()
{
	std::cout << "H\t->\t" << this->H << std::endl << "W\t->\t" << this->W << std::endl << "nH\t->\t" << this->nH << std::endl << "nW\t->\t" << this->nW << std::endl << "nN\t->\t" << this->nN << std::endl << "nE\t->\t" << this->nE << std::endl;
	std::cout << "mTbegin\t->\t" << this->mTbegin << std::endl << "mTime\t->\t" << this->mTime << std::endl << "mdTime\t->\t" << this->mdTime << std::endl << "mC\t->\t" << this->mC << std::endl << "mK\t->\t" << this->H << std::endl << "mR\t->\t" << this->mR << std::endl;
}

double GlobalData::getH() const
{
    return H;
}

double GlobalData::getW() const
{
    return W;
}

uint GlobalData::getNH() const
{
    return nH;
}

uint GlobalData::getNW() const
{
    return nW;
}

uint GlobalData::getNN() const
{
    return nN;
}

uint GlobalData::getNE() const
{
    return nE;
}

double GlobalData::getTemp()
{
	return this->mTbegin;
}


