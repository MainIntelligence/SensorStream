#pragma once
//Blurrer: basically used to remove high frequencies from a linear signal.
//	note this intended for use on an audio stream, such as a microphone, to get rid of unwanted noise.

template <unsigned int radi, typename T, typename SumT = T>
struct Blurrer {
	static constexpr int totalSums = 2*radi + 1;
	SumT sums [totalSums] {};
	T * pv;
	unsigned int count = 0;
	unsigned int len;
	Blurrer(T * pv, unsigned int total) : pv{pv}, len{total} {}
	void Blur();
private:
	void ShiftSums();
	void BeginBlur();
	void EndBlur();
	void FillSums();
	void RecalcShift();
};

template <unsigned int radi, typename T, typename SumT>
void Blurrer<radi, T, SumT>::RecalcShift() {
	for (unsigned int sumi = 0; sumi < radi ; sumi++) { 
		*(pv++) = sums[sumi] / totalSums;
	}
	count += radi;
};

//Fill the second half of the sum table with updated values.
//	Assumes: current middle sum (radi) corresponds to blur centroid radi + 1 relative to pv, last to pv + 2 * radi
template <unsigned int radi, typename T, typename SumT>
void Blurrer<radi, T, SumT>::FillSums() {
	unsigned int i = 0; unsigned int sumi = radi + 1;
	for (; sumi < totalSums ; sumi++) { sums[sumi] = sums[sumi - 1] + *(pv + totalSums + i) - *(pv + i); i++; }
}//pv[0] is subtracted from sums[radi], which is sums[2*radi] before shift, pv best be shifted over by radi before calling ShiftSum

//Shift the current right half (>= radi == middle) to the left half, not filling the right half with new values
template <unsigned int radi, typename T, typename SumT>
void Blurrer<radi, T, SumT>::ShiftSums() {
	for(unsigned int i = radi; i < 2 * radi + 1 ; i++) { sums[i - radi] = sums[i]; }
}


template <unsigned int radi, typename T, typename SumT>
void Blurrer<radi, T, SumT>::BeginBlur() {
	unsigned int i = 1;
	unsigned int sumi = 1;
	sums[0] = *pv;
	for (; i < radi + 1 ; i++ ) { sums[0] += *(pv + i); }
	for (; sumi < radi + 1 ; sumi++) { sums[sumi] = sums[sumi - 1] + *(pv + i); i++; }
	//i == 2 * radi + 1

	FillSums();
	//no longer need up to pv + radi - 1 inclusive
	for (sumi = 0; sumi < radi ; sumi++) { *(pv++) = sums[sumi] / (radi + sumi + 1); }
	
	ShiftSums();
	FillSums();
	count += radi;
}

template <unsigned int radi, typename T, typename SumT>
void Blurrer<radi, T, SumT>::EndBlur() {
	RecalcShift();
	ShiftSums();
	//must shift the last radi
	//len - count - radi   sums left that can be blurred.
	//	len - count values without a sum.
	unsigned int i = len - count - radi; //first bad sum.  // i - count > 0
	unsigned int j = 0;
	for (; i < len - count ; i++) { sums[i] = sums[i - 1] - *(pv + j); j++; }
	
	i = 0;
	for (; i < len - count - radi ; i++) { *(pv++) = sums[i] / totalSums; }
	unsigned int divider = totalSums - 1;
	for (; i < len - count ; i++) { *(pv++) = sums[i] / divider; divider--; }
}

template <unsigned int radi, typename T, typename SumT>
void Blurrer<radi, T, SumT>::Blur() {
	BeginBlur();

	for ( ; len > count + radi + totalSums ; ) {
		RecalcShift();
		ShiftSums();
		FillSums();
	}
	
	EndBlur();
};
