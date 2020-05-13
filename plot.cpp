/***************************************************************************
 *   Copyright (C) 2020 by Nuno Cardoso                                    *
 *   nmrcardoso@gmail.com                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


//#define USE_FL_WIDGET_DRAW

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <limits>
#include <cstdlib>
#include <algorithm>
#include <cstring>
#include <cassert>
#include <map>
#include <vector>
#include <cmath>
#include <float.h>

#include "plot.h"

#include "log.h"

#ifndef uint
#define uint unsigned int
#endif


using namespace std;



    
bool IsFiniteNumber(double x) {
    return (x <= DBL_MAX && x >= -DBL_MAX); 
}  



DrawPlot::DrawPlot(int x, int y, int w, int h, std::string name): Fl_Widget(x,y,w,h), plotname(name){
	bground = FL_LIGHT2;
	bcolor = FL_LIGHT3;
	fitRcolor = FL_LIGHT1;
	IntPointsColor = FL_RED;
	ExtPointsColor = FL_BLUE;
	Errorlinewidth = 2;
	linewidth = 2;
	dxy = 4;
	error = false;
	axislabelx = "";
	axislabely = "";
	optimzeX = true;
	optimzeY = true;
	setboundsX = false;
	setboundsY = false;
}
void DrawPlot::setError(){
	error = true;
}
void DrawPlot::unsetError(){
	error = false;
}
void DrawPlot::setLineWidth(int size){
	linewidth = size;
}

void DrawPlot::PlotName(std::string name){
	plotname = name;
}

void DrawPlot::setBoundsY(double _ymin, double _ymax){ 
	setboundsY = true;
	ymin = _ymin;
	ymax = _ymax;
}
void DrawPlot::setBoundsX(double _xmin, double _xmax){ 
	setboundsX = true;
	xmin = _xmin;
	xmax = _xmax;
}
void DrawPlot::UnsetBoundsY(){ 
	setboundsY = false;
}
void DrawPlot::UnsetBoundsX(){ 
	setboundsX = false;
}
	bool setboundsX, setboundsY;

void DrawPlot::axislabel(std::string labelx, std::string labely){
	axislabelx = labelx;
	axislabely = labely;
}


void DrawPlot::setErrorLineWidth(int size){
	Errorlinewidth = size;
}
void DrawPlot::setPointSize(int size){
	dxy = size;
}

void DrawPlot::setPointsColor(Fl_Color IntPoints, Fl_Color ExtPoints){
	IntPointsColor = IntPoints;
	ExtPointsColor = ExtPoints;
}
void DrawPlot::setBackgroundColor(Fl_Color color){
	bground = color;
}
void DrawPlot::setBackColor(Fl_Color color){
	bcolor = color;
}

void DrawPlot::setFRangeColor(Fl_Color color){
	fitRcolor = color;
}



template<typename type>
void fontscreenbox(type val, Fl_Font fontname, int fontsize, int &wi, int &hi){
	wi=0, hi=0;
	stringstream s; 
	s << val;
	fl_font(fontname, fontsize);
	fl_measure(s.str().c_str(), wi, hi);
}


void DrawPlot::add(DataLine data_, Fl_Color color, std::string name, DrawPlotMode _mode){
	datas.push_back(data_);
	data_color.push_back(color);
	label.push_back(name);
	mode.push_back(_mode);
}

void DrawPlot::draw() {
	fl_push_clip( x(), y(), w(), h() );
	createdraw();	
	fl_pop_clip();	
}
void DrawPlot::clear(){
	label.clear();
	data_color.clear();
	datas.clear();
}

void DrawPlot::OptimizeAxis(bool xaxis, bool yaxis){
	optimzeX = xaxis;
	optimzeY = yaxis;

}

void DrawPlot::createdraw(){
	if( datas.size() < 1 ) return;
	 	
	if(!setboundsX){
		xmin = 1e30;
		xmax = -1e30;
	}
	if(!setboundsY){
		ymin = 1e30;
		ymax = -1e30;
	}
	//Calculate the max and min of the data
	for(int k = 0; k < datas.size(); k++){
		DataLine data = datas.at(k);	
		for( DataLine::iterator it = data.begin(); it != data.end(); ++it ) {
			if(!setboundsY){
				if(error){
					ymin = std::min( ymin, it->y - it->error );
					ymax = std::max( ymax, it->y + it->error );
				}
				else{
					ymin = std::min( ymin, it->y );
					ymax = std::max( ymax, it->y );		
				}
			}
			if(!setboundsX){
				xmin = std::min( xmin, it->x );
				xmax = std::max( xmax, it->x );
			}
		}
	}
	//Calculate best max and min of the data
	int newbinsX = 5;
	if(optimzeX && !setboundsX) OptimizeTicksLimits(5, newbinsX, xmin, xmax, false);
	int newbinsY = 5;
	//needs this before draw area and call plot for the best number of ticks
	if(optimzeY && !setboundsY) OptimizeTicksLimits(8, newbinsY, ymin, ymax, false);
	setTicks(newbinsX, 5, newbinsY, 5);

	// Calculate the best space around plot
	fl_font(getTickFont(), getTickFontSize()+5);
	int border = 15;
	//int border = 55;
	int borderB = border;
	int borderT = border;
	int borderL = border;
	int borderR = border;
	if( plotname.size() > 0 ) borderT += fl_height();
	int w0=0, h0=0, w1=0, h1=0;
	int x0 = 0, x1 = 0;
	int y0 = 0, y1 = 0;
	if(xaxis){
		// Only needs to calculate using the size of first and last labels
		fontscreenbox(xmin, getTickFont(), getTickFontSize(), w0, h0);
		fontscreenbox(xmax, getTickFont(), getTickFontSize(), w1, h1);
		y0 = std::max( h0, h1);
		borderL = std::max( borderL, w0/2);
		borderR = std::max( borderR, w1/2);
		borderB = std::max( borderB, int(h1*1.5));
	}
	if(yaxis){
		// Needs to calculate the size of all tick labels on y axis
		yticks.CalculateTicksValues(newbinsY, ymin, ymax);
		for(int i = 0; i < yticks.NumTicksLvl1; ++i){
			fontscreenbox(yticks.TicksValuesLvl1[i], getTickFont(), getTickFontSize(), w0, h0);
			x0 = std::max( x0, w0);
		}
		borderB = std::max( borderB, h0/2);
		borderT = std::max( borderT, h0/2);
	}
	if(axislabelx.size() > 0){
		fl_font(getTickFont(), getTickFontSize());
		borderB += fl_height();
	}
	if(axislabely.size() > 0){
		fl_font(getTickFont(), getTickFontSize());
		borderL += fl_height();	
	}
	
	w_x0 = x()+borderL+x0;
	w_x1 = x()+w()-borderR;
	w_y0 = y()+borderT;
	w_y1 = y()+h()-borderB;
	int w_w = w_x1-w_x0;
	int w_h = w_y1-w_y0;
	//Init ticks
	InitTicks(w_x0, w_x1, w_y0, w_y1);

	double xrange = (xmax-xmin), yrange = (ymax-ymin);
	fAxisLength_x = w_x1-w_x0;
	fAxisLength_y = w_y1-w_y0;
	double factorx = fAxisLength_x/xrange;
	double factory = fAxisLength_y/yrange;

	//background color around the plot
	fl_color(bcolor);
	fl_rectf(x(), y(), w(), h());
	
	//background color inside the plot
	fl_line_style(0, 1);
	fl_color(bground);
	fl_rectf(w_x0, w_y0, w_w, w_h);

	//plot title
	if( plotname.size() > 0 ){
		fl_font(getTickFont(), getTickFontSize()+5);
		fl_color(FL_BLACK);
		int fx, fy;
		fl_measure(plotname.c_str(), fx, fy);	
	 	fl_draw(plotname.c_str(), x()+w()/2 - fx/2, y(), fx, fl_height(), FL_ALIGN_LEFT);
	}
	//x axis label
	if(axislabelx.size() > 0){
		fl_color(FL_BLACK);
		fl_font(getTickFont(), getTickFontSize());
		int fx, fy;
		fl_measure(axislabelx.c_str(), fx, fy);	
	 	fl_draw(axislabelx.c_str(), w_x0 + w_w/2 - fx/2, y()+h() - fl_height(), fx, fl_height(), FL_ALIGN_LEFT);
	}
	//y axis label
	if(axislabely.size() > 0){
		fl_color(FL_BLACK);
		fl_font(getTickFont(), getTickFontSize());
		int fx = 0, fy = 0;
		fl_measure(axislabely.c_str(), fx, fy);
	 	fl_draw(90, axislabely.c_str(), x() + fl_height(), w_y0+w_h/2 + fx/2);
 	}



	//Draw axis and ticks of the plot
	calculate(xmin, xmax, ymin, ymax);
	drawAxis();
	

	//Draw data points/lines
	for(int k = 0; k < datas.size(); k++){
		DataLine data = datas.at(k);
		Fl_Color color = data_color.at(k);
		bool plotpoint = false;
		bool plotline = false;
		if( mode.at(k) == POINTS ) plotpoint = true;
		if( mode.at(k) == LINES ) plotline = true;
		if( mode.at(k) == POINT_LINES ){
			plotpoint = true;
			plotline = true;
		}
		//DRAW POINTS
		for( uint i = 0; i < data.size(); ++i ) {
			double xx = data[i].x;
			int x = round(w_x0+(xx-xmin)*factorx);
			double yy = data[i].y;
			int y = round(w_y1-(yy-ymin)*factory);
			double yerror = data[i].error;
			if(isnan(yy) || isnan(error)) continue;			
			
			fl_color(color);			
			if(error && yerror){
				fl_line_style(0, Errorlinewidth);
				int posy0 = y - yerror * factory;
				int posy1 = y + yerror * factory;
				int posy = y;
				if(setboundsY ){				
					if( posy1 >= w_y1 ) posy1 = w_y1-1;
					else if( posy1 > w_y0) fl_line(x - dxy, posy1, x + dxy, posy1);
					if( posy0 <= w_y0 ) posy0 = w_y0+1;
					else if( posy0 < w_y1)  fl_line(x - dxy, posy0, x + dxy, posy0);
					
					//if(posy1 < w_y1 && posy0 > w_y0){
					if(posy1 > w_y0 && posy0 < w_y1){
						fl_line(x, posy0, x, posy1);
					}
				}
				else{
					//draw error bar
					fl_line(x, posy1, x, posy0);
					////draw error bar top bar			
					fl_line(x - dxy, posy1, x + dxy, posy1);
					////draw error bar bottom bar
					fl_line(x - dxy, posy0, x + dxy, posy0);
				}
			}		
			//draw square point
			fl_line_style(0, 1);
			if(plotpoint){
				if(setboundsY ){
					if(  y+dxy < w_y1 && y-dxy > w_y0 ) 
						fl_rectf(x - dxy, y-dxy, 2*dxy+1, 2*dxy+1 );			
				}
				else {
					fl_rectf(x - dxy, y-dxy, 2*dxy+1, 2*dxy+1 );
				}
			}
				
			if(plotline){
				fl_line_style(0, linewidth);
				if( i == 0 ) continue;
				fl_line(w_x0+rint((data[i-1].x-xmin)*factorx), w_y1-rint((data[i-1].y-ymin)*factory),  x, y);
			}
		}
	}
	//plot legend
	int maxlengthlabelx = 0;
	int maxlengthlabely = 0;
	int lx0 = 0, lx1 = 0, ly0 = 0, ly1 = 0;
	int nl = 0;
	for(int k = 0; k < datas.size(); k++){	
		if(label.at(k).size() < 1 ) continue;	
		fl_font(getTickFont(), getTickFontSize());
		int wi=0, hi=0, n=4;
		fl_measure(label.at(k).c_str(), wi, maxlengthlabely);
		maxlengthlabelx = std::max(maxlengthlabelx, wi);
		nl++;
	}
	lx0 = maxlengthlabelx + 5*maxlengthlabely/2;
	ly0 = (maxlengthlabely+ maxlengthlabely/2)*nl+ maxlengthlabely/2;
	lx1 = w_x1-lx0-ticks_length;
	ly1 = w_y0 + ticks_length;
	fl_rectf(lx1-maxlengthlabely/2, ly1, lx0, ly0, FL_LIGHT1 );
	nl=0;
	for(int k = 0; k < datas.size(); k++){
		if(label.at(k).size() < 1 ) continue;	
		ly1 +=maxlengthlabely/2;		
		fl_color(data_color.at(k));		
		fl_line_style(0, 1);
		if( mode.at(k) == POINTS || mode.at(k) == POINT_LINES) 
			fl_rectf(lx1, ly1+maxlengthlabely/4, maxlengthlabely/2, maxlengthlabely/2 );
		fl_line_style(0, linewidth);
		if( mode.at(k) == LINES || mode.at(k) == POINT_LINES) 
				fl_line(lx1-maxlengthlabely/2, ly1+maxlengthlabely/2, lx1+maxlengthlabely, ly1+maxlengthlabely/2);		
		fl_color(FL_BLACK);
		fl_font(getTickFont(), getTickFontSize());
	 	fl_draw(label.at(k).c_str(), lx1 + 3*maxlengthlabely/2, ly1, maxlengthlabelx, maxlengthlabely, FL_ALIGN_LEFT);
		ly1 +=maxlengthlabely;
		++nl;
	}	
}



