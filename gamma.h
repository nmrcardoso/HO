

#ifndef H__GAMMA
#define H__GAMMA

extern double gamma(double Z);
extern long double log_gamma(double N);
extern double approx_gamma(double z);
extern double approx_log_gamma(double Z);




/*
	Returns the Natural Logarithm of the Incomplete Gamma Function
	
	I converted the ChiSqr to work with Logarithms, and only calculate 
	the finised Value right at the end.  This allows us much more accurate
	calculations.  One result of this is that I had to increase the Number 
	of Iterations from 200 to 1000.  Feel free to play around with this if 
	you like, but this is the only way I've gotten to work.  
	Also, to make the code easier to work it, I separated out the main loop.  
*/

long double log_igf(long double S, long double Z);

long double KM(long double S, long double Z);


/*
	Incomplete Gamma Function

	No longer need as I'm now using the log_igf(), but I'll leave this here anyway.
*/

double igf(double S, double Z);



#endif

