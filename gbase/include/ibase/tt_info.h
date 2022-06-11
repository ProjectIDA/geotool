/*
 * tt_info.h include file.  Prototype definitions for radial_2D 
 * routines called outside of libloc.  
 *
 * SccsId[] = "@(#)tt_info.h	1.2 10/08/97 Copyright 1997 SAIC";
 */

#ifndef TT_INFO_H
#define TT_INFO_H

#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include"libloc.h"
#define KM_PER_DEG 		111.195
#define CHECK_MALLOC(p,t,c) 	if ((p=(t *)malloc((unsigned)(c)*sizeof(t)))==(t *)(NULL)) return HYerror4  
#define HYDRO_INDEX	1
#define INFRA_INDEX	2


typedef struct {
   	float		azi; 
   	int 		nrad;
   	float 		delta_rad;
   	float		*tt;
	float		*tt_error; }	Azimuth_Info;

typedef struct {
        char		*sta;
   	float 		lat;
   	float 		lon;
        char            *file;
        int		num_azimuth; 
   	Azimuth_Info 	*azimuth; } 	Station_Info;

typedef struct {
        char		*per;
	int		num_station;
   	Station_Info 	*station; } 	Period_Info;

typedef struct {
	double 		H_T_convert;
	int		current_hydro_per_index;
	int		current_infra_per_index;
	Bool		initiated;
	Bool		epoch_time_set;
	Bool		use_hydro_2D_table;
	Bool		use_infra_2D_table;
	Bool		informed; 
	int		current_tech_index;} 	Control_Flags;	

typedef struct {
	int		last_doy;
	int		index; }	Period_Time;

int read_HY_info(char *sta_file_path);

int initialize_Hydro_tt_tables (char *sta_names[], int num_sta_names, char *path);

int initialize_radial_2D_tables(char *sta_names[], int num_sta_names, char *tt_path);


Period_Info *get_hy_tt_pointer(void);
Period_Info *get_infra_tt_pointer(void);

void get_acoustic_tt(char *sta, double ev_lat,double ev_lon,double *travel_time,double *model_error,int *blocked);
 
int load_acoustic_tt(Period_Info* period, int i, int j);
 
void look_up_ttime(int station_index,float azimuth, float distance,double *travel_time,double *model_error,int *blocked,int tech_index);

int station_in_radial_2D_tables(char *sta);

void set_epoch_travel_time(double epoch);

char *get_current_radial_2D_period_name(char *sta);

double get_2D_model_error(char *sta, double dist, double azimuth);

void free_whole_table(Period_Info *period);

double get_H_T_convert(void);

#endif
