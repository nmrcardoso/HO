

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <map>

#include <random>
#include <algorithm>    // std::random_shuffle

#include <complex>
#include <vector>



#include "PostScript.H"  // Needs to be before Fl includes

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Chart.H>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Hor_Value_Slider.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Float_Input.H>
//#include <FL/Fl_run.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Select_Browser.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Choice.H>
#include <FL/fl_ask.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Printer.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Shared_Image.H>


#include <FL/Fl_Gl_Window.H>
#include <FL/gl_draw.H>
#include <FL/glut.H>
#include <FL/glu.h>
#include <FL/gl.h>


#include <time.h>
#include <numeric>
#include <vector>
#include <cmath>

#include "gamma.h"
#include "timer.h"
#include "ticks.h"
#include "plot.h"
#include "save_image.h"


using namespace std;


















uniform_real_distribution<double> ran1(-1,1);
uniform_real_distribution<double> ran2(0,1);

std::string filename_save[2];


static void save_win2(Fl_Widget *widget, void *data) {
	Fl_Window *win = (Fl_Window *)widget;				
	/*DrawPlot **wgt = (DrawPlot**)data;		
	std::string filename = "myHO.png";
	Image im(filename.c_str(), wgt, 2);
	std::cout << "Saving actual plot in: " << filename << std::endl;
	im.write(Image::PNG);*/	
	if (Fl::event() == FL_CLOSE || Fl::event_key()==FL_Escape){
		DrawPlot **plot = (DrawPlot**)data;
		Image im(filename_save[0].c_str(), plot[0]->w(), plot[0]->h(), plot[0]);
		std::cout << "Saving actual plot in: " << filename_save[0] << std::endl;
		im.write(Image::PNG); 
		Image im1(filename_save[1].c_str(), plot[1]->w(), plot[1]->h(), plot[1]);
		std::cout << "Saving actual plot in: " << filename_save[1] << std::endl;
		im1.write(Image::PNG);
		win->hide();
		exit(0);
	}
}
static void save_win1(Fl_Widget *widget, void *data) {
	Fl_Window *win = (Fl_Window *)widget;			
	if (Fl::event() == FL_CLOSE || Fl::event_key()==FL_Escape){
		DrawPlot *plot = (DrawPlot*)data;
		Image im(filename_save[0].c_str(), plot->w(), plot->h(), plot);
		std::cout << "Saving actual plot in: " << filename_save[0] << std::endl;
		im.write(Image::PNG); 
		win->hide();
		exit(0);
	}
}

//https://www.codeproject.com/Articles/432194/How-to-Calculate-the-Chi-Squared-P-Value
double chisqr(int Dof, double Cv){
    if(Cv < 0 || Dof < 1){
        return 0.0;
    }
	double K = ((double)Dof) * 0.5;
	double X = Cv * 0.5;
	if(Dof == 2){
		return exp(-1.0 * X);
	}
	long double PValue, Gam;
    long double ln_PV;
    ln_PV = log_igf(K, X);

    Gam = approx_gamma(K);
    //Gam = lgammal(K);
    //Gam = log_gamma(K);

    ln_PV -= Gam;
    PValue = 1.0 - expl(ln_PV);

	return (double)PValue;

}

#define nplots 5


class HO {
	Fl_Window *window;
	DataLine data, data1;
	DrawPlot *plot[nplots];
    double tmili;
    
    double *x_old, *x_new;
    vector<double> *x0xt;
    vector<double> x0xt_sum;
    vector<double> gamma;
    
    double c0, c1, c2, rho, tau, miter;
	default_random_engine seed;
    double accept_ratio;
    double a, w, T, delta, m;
    int nSize, iter,niter,A, meas, nsweep;
    double xsq_mean[2];
    vector<double> xsq_mean0;
    double err0;
    

    bool user_hit_abort_button() {
		if (Fl::event_key()==FL_Escape ) return true;
		return false;
	}
	
	double jackerr(const vector<double>& values){
		auto len = values.size();
		double s1 = 0, s2 = 0;
		for( double x : values ) {
			s1 += x;
			s2 += x*x;
		}
		double sumres = s2 - s1 * s1 / double(len);
		return std::sqrt(sumres/double(len*(len-1)));
	}

	double jackerr1(const vector<double>& values, const double mean){
		auto len = values.size();
		double s1 = 0, s2 = 0;
		for( double x : values ) {
			s1 = x-mean;
			s2 += s1*s1;
		};
		return std::sqrt(s2/double(len*(len-1)));
	}
	
	double JackerrTrials(const vector<double>& trials){
		auto len = trials.size();
		double s1 = 0, s2 = 0;
		for( double x : trials ) {
			s1 += x;
			s2 += x*x;
		}
		double sumres = s2 - s1 * s1 / len;
		return std::sqrt((len - 1) * sumres / len);
	}
	
	

	double jackerrTrials(std::vector<double> const & trials){
		double mean = std::accumulate(trials.begin(), trials.end(), 0.0) / trials.size();
		double sq_sum = std::inner_product(trials.begin(), trials.end(), trials.begin(), 0.0,
		[](double const & x, double const & y) { return x + y; },
		[mean](double const & x, double const & y) { return (x - mean)*(y - mean); });
		return std::sqrt(double(trials.size() - 1) * sq_sum/double(trials.size()));
	}
	double jackerrTrials(std::vector<double> const & trials, const double mean){
		double sq_sum = std::inner_product(trials.begin(), trials.end(), trials.begin(), 0.0,
		[](double const & x, double const & y) { return x + y; },
		[mean](double const & x, double const & y) { return (x - mean)*(y - mean); });
		return std::sqrt(double(trials.size() - 1) * sq_sum/double(trials.size()));
	}
	
	
	// Plot data
    void drawG0(){    
		  plot[0]->clear();
          string label = "Accept_ratio = " + ToString(100. * accept_ratio/(double(iter - 1)*double(nSize))) + "%";
          plot[0]->add(data, FL_BLUE, label.c_str());
		  plot[0]->axislabel("iter", "x(0)x(0)");
		  plot[0]->redraw();
	}
	
    void drawG1(){    	
		err0 = jackerr(x0xt[0]);
		string label = "E0: " + ToString(x0xt_sum.at(0)/double(meas)) + " ± " + ToString(err0);          
		DataLine data_plot;
		for(int i =0;i<x0xt[0].size();++i) {	
			DataPoint p;
			p.x = i;
			p.y = x0xt[0].at(i);
			data_plot.push_back(p);
		}
		plot[1]->clear();
		plot[1]->add(data_plot, FL_BLUE,label.c_str());
		plot[1]->axislabel("#configuration", "x(0)x(0)");
		plot[1]->redraw();
	}
	
    void drawG2(){  	
		plot[2]->clear();
		DataLine data3;
		for(int i =0;i<x0xt_sum.size();++i) {			
			DataPoint p;
			p.x = i;
			p.y = x0xt_sum.at(i)/double(meas);	//average
			p.error = jackerr(x0xt[i]);
			data3.push_back(p);		
		} 
		plot[2]->add(data3, FL_BLUE);
		plot[2]->axislabel("t", "<x(t)x(0)>");
		plot[2]->redraw();
			
		//plot -log(<x(t+1)x(0)>/<x(t)x(0)>)/a
		// error using Jackknife method
		DataLine data2;	
		int totalPoints = 100;
		if(totalPoints > (x0xt_sum.size() - 1) ) totalPoints = x0xt_sum.size()-1;
		for(int i =0;i < totalPoints;++i) {			
			if(	meas != x0xt[i].size() ){
				cout << "meas and array size does not have the same same.\nError..." << endl;
				exit(1);
			}
			DataPoint p;
			p.x = i;
			p.y = -std::log(x0xt_sum.at(i+1)/x0xt_sum.at(i))/a;	
			// x0xt_sum is not normalized, is just the sum 	
			double Jsum = 0.;
			for(int j=0;j<meas;++j){
				double tmp1 = x0xt_sum.at(i)*(x0xt_sum.at(i+1) -x0xt[i+1].at(j));
				double tmp2 = x0xt_sum.at(i+1)*(x0xt_sum.at(i)-x0xt[i].at(j));
				double tmp3 = -std::log(tmp2/tmp1)/a;
				Jsum += tmp3 * tmp3;
			}
			p.error = std::sqrt(Jsum*double(meas-1)/double(meas));
			data2.push_back(p);		
		}
		plot[3]->clear();
		plot[3]->setBoundsY(0., 2);
		string label = "-log(<x(t+1)x(0)>/<x(t)x(0)>)/a";		
		plot[3]->add(data2, FL_BLUE, label.c_str());	
		plot[3]->axislabel("t", "Effective Energy Gap");	
		
		
		//Constant Fit...
		double DeltaE = 0.;
		double error = 0.;
		double chi2dof = 1000.;
		double pvalue = 0.;
		int posi = 0;
		int posf = 0;
		
		int maxlength = data2.size();
		for(int i = 2; i < data2.size(); ++i){
			DataPoint p = data2.at(i);
			if( isnan(p.x) || isnan(p.y) || isnan(p.y) ) break;
			maxlength = i;
		}		
		
		for(int maxl = 2; maxl < maxlength; ++maxl){
			for(int minl = 0; minl < maxl; ++minl){
				if( (maxl-minl) < 2 ) continue;				
				double s0 = 0, s1 = 0;
				for(int i =minl;i<maxl;++i) {
					DataPoint p = data2.at(i);
					s0 += 1./(p.error *p.error);
					s1 += p.y / (p.error *p.error);			
				}
				double DeltaE1 = s1/s0;
				vector<double> trials;
				double s2 = 0;
				for(int i =minl;i<maxl;++i) {
					DataPoint p = data2.at(i);
					double err2 = p.error * p.error;
					trials.push_back( (s1 - p.y/err2) / (s0 - 1./err2) );
					s2 += pow( (p.y - DeltaE1) / p.error, 2 );					
				}
				double error1 = JackerrTrials(trials);	  
				double chi2dof1 = s2 / (trials.size() - 1);
				double pvalue0 = chisqr(trials.size() - 1, s2);
								
				if(std::abs(1.-chi2dof1) < std::abs(1.-chi2dof) && pvalue0 > pvalue) {
					if(minl < 20 ){
						DeltaE = DeltaE1;
						error = error1;
						chi2dof = chi2dof1;
						pvalue = pvalue0;
						posf = maxl;
						posi = minl;
					}
				}
			}
		}	
		//Plot fit line		
		DataLine datal;
		for(int maxl = posi; maxl <= posf; ++maxl){
			DataPoint p;
			p.x = maxl;
			p.y = DeltaE;
			p.error = 0.;
			datal.push_back(p);
		}
		label = "fit: DeltaE: " + ToString(DeltaE) + " ± " + ToString(error) + "   chi²/dof = " + ToString(chi2dof) + "  p-value = " + ToString(pvalue) + "\t[" + ToString(posi) + "," + ToString(posf) + "]";	
		plot[3]->add(datal, FL_RED, label, LINES);
		plot[3]->redraw();
	}
	
	
	double hist_dx;
    void drawG3(){    	   
		double sum = 0.;
		for(int i =0;i<data1.size();++i) sum += data1.at(i).error;
		sum *= hist_dx;
		for(int i =0;i<data1.size();++i) data1.at(i).y = data1.at(i).error/sum;
		plot[4]->clear();
		plot[4]->add(data1, FL_BLUE);
		plot[4]->axislabel("x", "|ψ|²");
		plot[4]->redraw();
	}
    bool calculation_done(){ if(iter > niter) return true;
    	return false;
    }
    // end plot data    
    
	static void win_cb (Fl_Widget *widget, void *userdata) {
		Fl_Window *win = (Fl_Window *)widget;
		HO *o = (HO *)userdata;
		if (Fl::event() == FL_CLOSE || o->user_hit_abort_button()){
			o->save();
			win->hide();
			exit(0);
		}
		Fl_Widget::default_callback(win, userdata);
	}

	void save(){
		if(0)for(int i = 0; i < 4; ++i){
			std::string filename = "HO_" + ToString(i) + ".png";
			int ww=plot[i]->w(), hh=plot[i]->h();
			Image im(filename.c_str(), ww, hh, plot[i]);
			std::cout << "Saving actual plot in: " << filename << std::endl;
			im.write(Image::PNG);
		}
		string fname = "ho.ps";
		std::cout << "Saving actual plot to: " << fname << std::endl;
		if(0){
			FILE * f = fopen(fname.c_str(),"w");
			if(!f) return;
			bool wid = true;
			Fl_PostScript_File_Device *driver = new Fl_PostScript_File_Device;
			int nn = nplots;
			int margin = 0;//20;
			int height = 0, width = plot[0]->w();
			for(int i = 0; i < nn; ++i){
				height += plot[i]->h();
				if(width < plot[i]->w()) width = plot[i]->w();
				if( i < nn-1) height+=margin;
			}
			driver->start_job_eps(f, 3, 0, 0, width, height);
			driver->start_page_eps();	
			fl_color(FL_BLACK);
			fl_line_style(FL_SOLID, 2);
			fl_font(FL_COURIER, 12);
			int height0 = 0;
			for(int i = 0; i < nn; ++i){
				driver->origin(0, height0);
				driver->print_widget(plot[i], 0, 0);
				height0 += plot[i]->h()+margin;		
			}
			driver->end_page();
			delete driver;
			fclose(f);
		}
		else{
			FILE * f = fopen(fname.c_str(),"w");
			if(!f) return;
			window->border(false); // remove window borders
			int width = window->decorated_w(); int height = window->decorated_h();
			Fl_PostScript_File_Device *driver = new Fl_PostScript_File_Device;
			driver->start_job_eps(f, 3, 0, 0, width, height );
			driver->start_page_eps();
			driver->print_window(window, 0, 0);
			driver->end_page();
			window->border(true); //set again window borders
			delete driver;
			fclose(f);
		}	
		string runcode = "ps2pdf14 " + fname;
		int result = system(runcode.c_str());
		runcode = "rm -f  " + fname;
		result = system(runcode.c_str());
	}
	


	void calculateCorTime(){
		static bool calc =  false;
		if(iter<1000 ) return ;
		if(calc) return;
		double avg = 0.;
		for(int i = 0; i < gamma.size();++i) avg += gamma.at(i);
		avg/=double(gamma.size());

		double rho=0.;
		for(int i = 0; i < gamma.size();++i) rho += (gamma.at(i)-avg)*(gamma.at(i)-avg);
		rho /= double(gamma.size());

		vector<double> corr;
		for(int j = 1; j < gamma.size();++j){
			double ga = 0.;
			for(int i = 0; i < gamma.size()-j;++i) ga += (gamma.at(i)-avg)*(gamma.at(i+j)-avg);
			ga /= double(gamma.size()-j);
			corr.push_back(ga/rho);
		}

		for(int i = 0; i < gamma.size()-1;++i){
			double tau_int = 0.5;
			for(int j = 0; j < i;++j) 
			tau_int += corr.at(j);
			if(i>int(4.*tau_int+1)){
				nsweep = i;
				std::cout << "iter: " << iter << "\tsweeps: " << nsweep << "\ttau: " << tau_int << endl;
				//exit(0);
				gamma.clear();
				calc = true;
				break;
			}
		}	
	}
	
	double expS (double x0, double x1, double x2){
		double S=(x1-x0)*(x1-x0)/(2.*a) + a*0.5*w*w*x1*x1 + (x2-x1)*(x2-x1)/(2*a);
		return exp(-S);
	}
	
	void evolve(){	
		for(int i = 0; i < nSize; ++i){
			x_new[i] = x_old[i] + delta*ran1(seed);
			double expS_new = expS( x_new[(i-1+nSize)%nSize], x_new[i], x_new[(i+1)%nSize]);
			double expS_old = expS( x_old[(i-1+nSize)%nSize], x_old[i], x_old[(i+1)%nSize]);
			if(ran2(seed) < expS_new/expS_old){
				x_old[i] = x_new[i];
				accept_ratio = accept_ratio +1.;
			}
			else{
				x_new[i] = x_old[i];
			}
		}
	}
	void termalize(int termN){
		while (iter < termN) {
			evolve();
			iter++;
			if (user_hit_abort_button()) break;
		}
		iter = 0;
	}



	void calculate(){
		using namespace std;
		while (iter<=niter) {
			Timer a0; a0.start();
			while(a0.getElapsedTimeInMilliSec() < tmili ){		
				evolve();
				double xsq0 = 0.;
				for(int i = 0; i < nSize; ++i) xsq0 += x_new[i]*x_new[i];
				xsq0/=double(nSize);
				gamma.push_back(xsq0);
				calculateCorTime();
				DataPoint p;
				p.x = iter;
				p.y = xsq0;
				data.push_back(p);				
				if((iter%nsweep)==0){
					xsq_mean[0] +=xsq0;
					xsq_mean0.push_back(xsq0);
					for(int j = 0; j < nSize; ++j){
						double tmp = 0.;
						for(int i = 0; i < nSize; ++i) tmp += x_new[i]*x_new[(i+j)%nSize];
						tmp/=double(nSize);
						x0xt[j].push_back(tmp);		
						
						if(meas==0) x0xt_sum.push_back(tmp);
						else x0xt_sum.at(j) += tmp;
					}		
					double dx = hist_dx/2.;
					for(int i = 0; i < nSize; ++i)
					for(int j = 0; j < data1.size()-1; ++j){
						if(x_new[i] >= data1.at(j).x - dx && x_new[i] < data1.at(j).x + dx){
							data1.at(j).error +=1.;
							break;
						}
					}			
					meas++;
				}
				iter++;
				if( iter > niter ) break;
			}
			int keepN = 2000;
			if(data.size() > keepN*2) data.erase(data.begin(), data.begin() + (data.size()-keepN));
			Fl::check();
			/*a0.stop();
			cout << "iter: " << iter << endl;
			cout << "calc time: " << a0.getElapsedTime() << endl;
			a0.start();*/
			drawG0();
			drawG1();
			drawG2();
			drawG3();
			/*a0.stop();
			cout << "plot time: " << a0.getElapsedTime() << endl;*/
			if (user_hit_abort_button()) break;
		}
	}
	
	public:
    HO(int W,int H,const char*L=0) {
    window = new Fl_Window(W,H, "Harmonic Oscillator");
		window->begin();		
    		window->callback( win_cb, this ); /*
   			int sy = rint((window->h()-40)/nplots);   
		   	plot[0] = new DrawPlot(20, 20, window->w()-40, sy);
		   	plot[1] = new DrawPlot(20, sy+20, window->w()-40, sy);
		   	plot[2] = new DrawPlot(20, 2*sy+20, window->w()-40, sy);
		   	plot[3] = new DrawPlot(20, 3*sy+20, window->w()-40, sy);
		   	plot[4] = new DrawPlot(20, 4*sy+20, window->w()-40, sy);*/
		   	
		   	int nrows = (nplots+1)/2;
		   	int ncols = (nplots)/2;
   			int sx = rint((window->w()-40)/ncols); 
   			int sy = rint((window->h()-40)/nrows);   
		   	plot[0] = new DrawPlot(20, 20, sx, sy);
		   	plot[1] = new DrawPlot(20, sy+20, sx, sy);
		   	plot[2] = new DrawPlot(sx+20, sy+20, sx, sy);
		   	plot[4] = new DrawPlot(sx+20, 20, sx, sy);
		   	plot[3] = new DrawPlot(20, 2*sy+20, window->w()-40, sy);
		window->end();
		window->resizable(window);
		window->show();
		for(int i = 0; i < nplots; ++i){
			plot[i]->setPointSize(2);
			plot[i]->setLineWidth(2);
			plot[i]->setErrorLineWidth(2);
			plot[i]->setAxisFontSize(14);
			plot[i]->setAxisX();
			plot[i]->setAxisY();
			plot[i]->setGridLines(1);
			plot[i]->OptimizeAxis(false, true);
		}
		plot[0]->setPointSize(1);
		plot[1]->setPointSize(1);
		plot[2]->setError();
		plot[3]->setError();
		
		hist_dx = 0.05;
		for(double i = -3; i <= 3; i+=hist_dx){
			DataPoint p;
			p.x = i;
			p.y = 0;
			p.error = 0;
			data1.push_back(p);
		}
		
		m=1.;
		a=0.1;
		w=1.;
		delta=2.*std::sqrt(a);
		T=20.*M_PI/w;
		nSize=round(T/a);
		std::cout << "m: " << m << std::endl;
		std::cout << "w: " << w << std::endl;
		std::cout << "a: " << a << std::endl;
		std::cout << "n: " << nSize << std::endl;
		std::cout << "delta: " << delta << std::endl;
		xsq_mean[0]=0.;
		xsq_mean[1]=0.;
		meas=0;
		x_old = new double[nSize]();
		x_new = new double[nSize]();
		x0xt = new vector<double>[nSize];
		iter = 0;
		niter=1000000000;
		A=0;
		tmili = 500;
		accept_ratio = 0.;
		nsweep = 100000000;
		seed.seed(time(NULL));
	
		termalize(1000);	
		calculate();
    } 
    
    

    ~HO(){ 
    	for(int i = 0; i < 4; ++i)	plot[i]->clear();
    	delete[] x0xt, x_old, x_new;
	}
};



int ho(int narg, char** args){
	HO *myHO = new HO(1280, 1024);
    return(Fl::run());
}








int main(int argc, char** argv)
{
	using namespace std;
	Timer a0;
	a0.start();

	ho( argc, argv);

	a0.stop();
	cout << "Time: " << a0.getElapsedTime() << endl;
}

