/* Created by Matt Greig, Robert Givan and Sungwook Yoon */
/* Modified by Blai Bonet */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

int gnum_x_coord = 2;
int gnum_y_coord = 2;

void print_domain() 
{
  printf("(define (domain drive-unrolled)\n");
  printf("  (:requirements :typing :probabilistic-effects :conditional-effects :equality)\n");
  printf("  (:types coord direction color delay preference length rotation)\n");
  printf("  (:predicates\n");
  printf("    (heading ?d - direction)\n");
  printf("    (clockwise ?d1 ?d2 - direction)\n");
  printf("    (at ?x - coord ?y - coord)\n");
  printf("    (nextx ?a - coord ?b - coord ?h - direction)\n");
  printf("    (nexty ?a - coord ?b - coord ?h - direction)\n");
  printf("    (light_color ?c - color)\n");
  printf("    (light_delay ?x ?y - coord ?d - delay)\n");
  printf("    (light_preference ?x ?y - coord ?p - preference)\n");
  printf("    (road-length ?start-x ?start-y ?end-x ?end-y - coord ?l - length)\n");
  printf("    (alive)\n");
  printf("  )\n");
  printf("  (:constants\n");
  printf("    left right straight - rotation\n");
  printf("    north south east west - direction\n");
  printf("    green red unknown - color\n");
  printf("    quick normal slow - delay\n");
  printf("    north_south none east_west - preference\n");
  printf("    short medium long - length\n");
  printf("  )\n");
  printf("  (:action look_at_light_north\n");
  printf("    :parameters (?x - coord ?y - coord)\n");
  printf("    :precondition (and (light_color unknown) (at ?x ?y) (heading north))\n");
  printf("    :effect\n");
  printf("      (and (probabilistic\n");
  printf("             9/10 (when (light_preference ?x ?y north_south)\n");
  printf("                    (and (not (light_color unknown)) (light_color green)))\n");
  printf("             1/10 (when (light_preference ?x ?y north_south)\n");
  printf("                    (and (not (light_color unknown)) (light_color red))))\n");
  printf("           (probabilistic\n");
  printf("             1/10 (when (light_preference ?x ?y east_west)\n");
  printf("                    (and (not (light_color unknown)) (light_color green)))\n");
  printf("             9/10 (when (light_preference ?x ?y east_west)\n");
  printf("                    (and (not (light_color unknown)) (light_color red)))))\n");
  printf("  )\n");
  printf("  (:action look_at_light_south\n");
  printf("    :parameters (?x - coord ?y - coord)\n");
  printf("    :precondition (and (light_color unknown) (at ?x ?y) (heading south))\n");
  printf("    :effect\n");
  printf("      (and (probabilistic\n");
  printf("             9/10 (when (light_preference ?x ?y north_south)\n");
  printf("                    (and (not (light_color unknown)) (light_color green)))\n");
  printf("             1/10 (when (light_preference ?x ?y north_south)\n");
  printf("                    (and (not (light_color unknown)) (light_color red))))\n");
  printf("           (probabilistic\n");
  printf("             1/10 (when (light_preference ?x ?y east_west)\n");
  printf("                    (and (not (light_color unknown)) (light_color green)))\n");
  printf("             9/10 (when (light_preference ?x ?y east_west)\n");
  printf("                    (and (not (light_color unknown)) (light_color red)))))\n");
  printf("  )\n");
  printf("  (:action look_at_light_east\n");
  printf("    :parameters (?x - coord ?y - coord)\n");
  printf("    :precondition (and (light_color unknown) (at ?x ?y) (heading east))\n");
  printf("    :effect\n");
  printf("      (and (probabilistic\n");
  printf("             1/10 (when (light_preference ?x ?y north_south)\n");
  printf("                    (and (not (light_color unknown)) (light_color green)))\n");
  printf("             9/10 (when (light_preference ?x ?y north_south)\n");
  printf("                    (and (not (light_color unknown)) (light_color red))))\n");
  printf("           (probabilistic\n");
  printf("             9/10 (when (light_preference ?x ?y east_west)\n");
  printf("                    (and (not (light_color unknown)) (light_color green)))\n");
  printf("             1/10 (when (light_preference ?x ?y east_west)\n");
  printf("                    (and (not (light_color unknown)) (light_color red)))))\n");
  printf("  )\n");
  printf("  (:action look_at_light_west\n");
  printf("    :parameters (?x - coord ?y - coord)\n");
  printf("    :precondition (and (light_color unknown) (at ?x ?y) (heading west))\n");
  printf("    :effect\n");
  printf("      (and (probabilistic\n");
  printf("             1/10 (when (light_preference ?x ?y north_south)\n");
  printf("                    (and (not (light_color unknown)) (light_color green)))\n");
  printf("             9/10 (when (light_preference ?x ?y north_south)\n");
  printf("                    (and (not (light_color unknown))(light_color red))))\n");
  printf("           (probabilistic\n");
  printf("             9/10 (when (light_preference ?x ?y east_west)\n");
  printf("                    (and (not (light_color unknown))(light_color green)))\n");
  printf("             1/10 (when (light_preference ?x ?y east_west)\n");
  printf("                    (and (not (light_color unknown))(light_color red)))))\n");
  printf("  )\n");
  printf("  (:action look_at_light_none\n");
  printf("    :parameters (?x - coord ?y - coord)\n");
  printf("    :precondition (and (light_color unknown) (at ?x ?y) (light_preference ?x ?y none))\n");
  printf("    :effect\n");
  printf("      (and (probabilistic\n");
  printf("             1/2 (and (not (light_color unknown)) (light_color green))\n");
  printf("             1/2 (and (not (light_color unknown)) (light_color red))))\n");
  printf("  )\n");
  printf("  (:action wait_on_light\n");
  printf("    :parameters (?x - coord ?y - coord)\n");
  printf("    :precondition (and (light_color red) (at ?x ?y))\n");
  printf("    :effect (and (probabilistic 1/100 (not (alive)))\n");
  printf("                 (probabilistic\n");
  printf("                   1/2 (when (light_delay ?x ?y quick)\n");
  printf("                         (and (not (light_color red)) (light_color green))))\n");
  printf("                 (probabilistic\n");
  printf("                   1/5 (when (light_delay ?x ?y normal)\n");
  printf("                         (and (not (light_color red))(light_color green))))\n");
  printf("                 (probabilistic\n");
  printf("                   1/10 (when (light_delay ?x ?y slow)\n");
  printf("                         (and (not (light_color red))(light_color green))))\n");
  printf("            )\n");
  printf("  )\n");
  printf("  (:action proceed-right\n");
  printf("    :parameters (?x ?y ?new-x ?new-y - coord ?old-heading ?new-heading - direction ?length - length)\n");
  printf("    :precondition (and (light_color green) (at ?x ?y) (heading ?old-heading) (clockwise ?old-heading ?new-heading) (nextx ?x ?new-x ?new-heading) (nexty ?y ?new-y ?new-heading) (road-length ?x ?y ?new-x ?new-y ?length))\n");
  printf("    :effect (and (not (light_color green))\n");
  printf("                 (light_color unknown)\n");
  printf("                 (probabilistic 1/50 (when (= ?length short) (not (alive))))\n");
  printf("                 (probabilistic 1/20 (when (= ?length medium) (not (alive))))\n");
  printf("                 (probabilistic 1/10 (when (= ?length long) (not (alive))))\n");
  printf("                 (not (heading ?old-heading))\n");
  printf("                 (heading ?new-heading)\n");
  printf("                 (not (at ?x ?y))\n");
  printf("                 (at ?new-x ?new-y))\n");
  printf("  )\n");
  printf("  (:action proceed-left\n");
  printf("    :parameters (?x ?y ?new-x ?new-y - coord ?old-heading ?new-heading - direction ?length - length)\n");
  printf("    :precondition (and (light_color green) (at ?x ?y) (heading ?old-heading) (clockwise ?new-heading ?old-heading) (nextx ?x ?new-x ?new-heading) (nexty ?y ?new-y ?new-heading) (road-length ?x ?y ?new-x ?new-y ?length))\n");
  printf("    :effect (and (not (light_color green))\n");
  printf("                 (light_color unknown)\n");
  printf("                 (probabilistic 1/50 (when (= ?length short) (not (alive))))\n");
  printf("                 (probabilistic 1/20 (when (= ?length medium) (not (alive))))\n");
  printf("                 (probabilistic 1/10 (when (= ?length long) (not (alive))))\n");
  printf("                 (not (heading ?old-heading))\n");
  printf("                 (heading ?new-heading)\n");
  printf("                 (not (at ?x ?y))\n");
  printf("                 (at ?new-x ?new-y))\n");
  printf("  )\n");
  printf("  (:action proceed-straight\n");
  printf("    :parameters (?x ?y ?new-x ?new-y - coord ?heading - direction ?length - length)\n");
  printf("    :precondition (and (light_color green) (at ?x ?y) (heading ?heading) (nextx ?x ?new-x ?heading) (nexty ?y ?new-y ?heading) (road-length ?x ?y ?new-x ?new-y ?length))\n");
  printf("    :effect (and (not (light_color green))\n");
  printf("                 (light_color unknown)\n");
  printf("                 (probabilistic 1/50 (when (= ?length short) (not (alive))))\n");
  printf("                 (probabilistic 1/20 (when (= ?length medium) (not (alive))))\n");
  printf("                 (probabilistic 1/10 (when (= ?length long) (not (alive))))\n");
  printf("                 (not (at ?x ?y))\n");
  printf("                 (at ?new-x ?new-y))\n");
  printf("  )\n");
  printf(")\n");
}

int in_the_route(int x, int y, int *route, int rwl) 
{
  int i;
  for(i=0; i< rwl; i++) {
    /*printf("%d %d %d %d %d \n", i , x, y, route[i*2], route[i*2 +1]);*/
    if( (x == route[i*2]) && (y == route[i*2 + 1]) ) {
      if( i == (rwl -1) )
	return i;
      if( (x != route[(i+1)*2]) || (y != route[(i+1)*2 + 1]) )
	return i;
      else
	continue;
    }
  }
  return -1;
}

void print_problem() 
{
  int i, j;
  int heading, delay, preference, length,dir, move;
  int rwl = ((gnum_x_coord -1 + gnum_y_coord -1) + (gnum_x_coord-1 + gnum_y_coord-1)/2);
  int *r_route = malloc(sizeof(int) * 2*rwl);
  int loc;
  int num_coord;
  

  r_route[0] = 0;
  r_route[1] = 0;
  /* first random route then construct */
  for(i = 1; i < rwl ; i++) {
    /*first decide direction*/
    dir = rand() % 2;
    if(dir) {
      /* x */
      move = rand() % 5;
      if(move < 3){
	if ((r_route[(i-1)*2] + 1) > (gnum_x_coord -1))
	  r_route[i*2] = gnum_x_coord-1;
	else	
	  r_route[i*2] = r_route[(i-1)*2] + 1;
	
	r_route[i*2+1] = r_route[(i-1)*2 + 1];
      } else {

	if ((r_route[(i-1)*2] -1) < 0)
	  r_route[i*2] = 0;
	else
	  r_route[i*2] = r_route[(i-1)*2] -1;
	
	r_route[i*2+1] = r_route[(i-1)*2 + 1];
      }
    } else {
      move = rand() % 5;
      if(move < 3){
	if( (r_route[(i-1)*2+1] + 1) > ( gnum_y_coord -1) )
	  r_route[i*2+1] =  gnum_y_coord -1;
	else
	  r_route[i*2+1] = r_route[(i-1)*2+1] + 1;
	
	r_route[i*2] = r_route[(i-1)*2];
	
      } else {
	if ( (r_route[(i-1)*2+1] -1) < 0 )
	  r_route[i*2+1] =  0;
	else
	  r_route[i*2+1] = r_route[(i-1)*2+1] -1;
	
	r_route[i*2] = r_route[(i-1)*2];
      }

    }
  }
  
  
  printf("(define (problem a-drive-unrolled-problem%d)\n", rand()%1000);
  printf("(:domain drive-unrolled)\n");
  printf("(:objects ");

  if (gnum_x_coord > gnum_y_coord)
    num_coord = gnum_x_coord;
  else
    num_coord = gnum_y_coord;
  
  for(i=0; i< num_coord; i++) 
    printf("c%d ", i);
  printf(" - coord)\n");

  printf("(:init \n");
  heading = rand() % 2;
  if( heading == 0 )
    printf("       (heading north)\n");
  if( heading == 1 )
    printf("       (heading east)\n");

  printf("       (at c0 c0)\n");
  printf("       (alive)\n");
  printf("       (light_color unknown)\n");

  printf("       (clockwise north east)\n");
  printf("       (clockwise east south)\n");
  printf("       (clockwise south west)\n");
  printf("       (clockwise west north)\n");
  
  for(i=0; i< gnum_x_coord-1; i++) {
    printf("       (nextx c%d c%d east)\n", i , i+1);
    printf("       (nextx c%d c%d west)\n", i+1 , i);
  }

  for(i=0; i< gnum_x_coord; i++) {
    printf("       (nextx c%d c%d north)\n", i, i);
    printf("       (nextx c%d c%d south)\n", i, i);
  }
  
  for(j=0; j< gnum_y_coord-1; j++) {
    printf("       (nexty c%d c%d north)\n", j , j+1);
    printf("       (nexty c%d c%d south)\n", j+1 , j);    
  }

  for(i=0; i< gnum_y_coord; i++) {
    printf("       (nexty c%d c%d east)\n", i, i);
    printf("       (nexty c%d c%d west)\n", i, i);
  }
  
  for(i=0; i< gnum_x_coord; i++) {
    for(j=0; j< gnum_y_coord; j++) {
      if( (in_the_route(i,j,r_route, rwl) > -1) )
	printf("       (light_delay c%d c%d quick)\n", i,j);
      else {
	
	delay = rand() % 3;
	if( delay == 0)
	  printf("       (light_delay c%d c%d slow)\n", i,j);
	if( delay == 1)
	  printf("       (light_delay c%d c%d normal)\n", i,j);
	if( delay == 2)
	  printf("       (light_delay c%d c%d quick)\n", i,j);
	
      }
      
    }
  }
  
  for(i=0; i< gnum_x_coord; i++) {
    for(j=0; j< gnum_y_coord; j++) {
      if( ((loc = in_the_route(i,j,r_route, rwl)) > -1) && (loc < rwl -1) ) {
	if( (i == r_route[(loc+1)*2]) && (j == r_route[(loc+1)*2+1]) ) {
	  printf("       (light_preference c%d c%d none)\n", i,j);
	  continue;
	}
	if( (i == r_route[(loc+1)*2]) ) {
	  printf("       (light_preference c%d c%d north_south)\n", i,j);
	  continue;
	}
	if( (j == r_route[(loc+1)*2+1]) ) {
	  printf("       (light_preference c%d c%d east_west)\n", i,j);
	  continue;
	}
	
      } else {
	preference = rand() % 3;
	if( preference == 0)
	  printf("       (light_preference c%d c%d east_west)\n", i,j);
	if( preference == 1)
	  printf("       (light_preference c%d c%d north_south)\n", i,j);
	if( preference == 2)
	  printf("       (light_preference c%d c%d none)\n", i,j);
      }
      
    }
  }

  
  for(i=0; i< gnum_x_coord; i++) {
    for(j=0; j< (gnum_y_coord -1); j++) {
      loc = in_the_route(i,j,r_route,rwl);
      
      if( (loc > -1) &&
	  ( ((loc < rwl-1) && (i == r_route[(loc+1)*2]) &&
	     ( (j+1) == r_route[(loc+1)*2 +1] )) ||
	    ((loc < rwl) && (i == r_route[(loc-1)*2]) &&
	     ( (j+1) == r_route[(loc-1)*2 +1] )) ) ) {
	  printf("       (road-length c%d c%d c%d c%d short)\n", i, j, i,j+1);
	  printf("       (road-length c%d c%d c%d c%d short)\n", i, j+1, i, j);
      } else {
	length = rand() % 3;
	if( length == 0 ) {
	  printf("       (road-length c%d c%d c%d c%d short)\n", i, j, i,j+1);
	  printf("       (road-length c%d c%d c%d c%d short)\n", i, j+1, i, j);
	}
	if( length == 1 ) {
	  printf("       (road-length c%d c%d c%d c%d medium)\n", i, j, i,j+1);
	  printf("       (road-length c%d c%d c%d c%d medium)\n", i, j+1, i, j);
	}
	if( length == 2 ) {
	  printf("       (road-length c%d c%d c%d c%d long)\n", i, j, i,j+1);
	  printf("       (road-length c%d c%d c%d c%d long)\n", i, j+1, i, j);
	}
      }
    }
  }

  for(j=0; j< gnum_y_coord; j++) {
    for(i=0; i< (gnum_x_coord -1); i++) {
      loc = in_the_route(i,j,r_route,rwl);
      if( (loc > -1) &&
	  ( ((loc < rwl-1) && (j == r_route[(loc+1)*2 +1]) &&
	     ( (i+1) == r_route[(loc+1)*2] )) ||
	    ((loc < rwl) && (j == r_route[(loc-1)*2 +1]) &&
	     ( (i+1) == r_route[(loc-1)*2] )) ) ) {
	printf("       (road-length c%d c%d c%d c%d short)\n", i, j, i+1,j);
	printf("       (road-length c%d c%d c%d c%d short)\n", i+1, j, i, j);
      } else {
	length = rand() % 3;
	if( length == 0 ) {
	  printf("       (road-length c%d c%d c%d c%d short)\n", i, j, i+1,j);
	  printf("       (road-length c%d c%d c%d c%d short)\n", i+1, j, i, j);
	}
	if( length == 1 ) {
	  printf("       (road-length c%d c%d c%d c%d medium)\n", i, j, i+1,j);
	  printf("       (road-length c%d c%d c%d c%d medium)\n", i+1, j, i, j);
	}
	if( length == 2 ) {
	  printf("       (road-length c%d c%d c%d c%d long)\n", i, j, i+1,j);
	  printf("       (road-length c%d c%d c%d c%d long)\n", i+1, j, i, j);
	}
      }
    }
  }
  
  
  
  printf(")\n");
  
  printf("(:goal (and (alive) (at c%d c%d))))\n", gnum_x_coord-1, gnum_y_coord-1);
}


int process_command_line( int argc, char *argv[] )

{

  char option;
  
  while ( --argc && ++argv ) {
    if ( *argv[0] != '-' || strlen(*argv) != 2 ) {
      return 0;
    }
    option = *++argv[0];
    switch ( option ) {
    default:
      if ( --argc && ++argv ) {
	switch ( option ) {

	case 'x':
	  sscanf( *argv, "%d", &gnum_x_coord );
	  break;
	case 'y':
	  sscanf( *argv, "%d", &gnum_y_coord);
	  break;
	  
	default:
	  printf( "\ndrive: unknown option: %c entered\n\n", option );
	  return 0;
	}
      } else {
	return 0;
      }
    }
  }

  return 1;

}

int main(int argc, char *argv[]) 
{

 
 
  
  /* randomize */
  srand( (unsigned int)getpid() );

   /* read in command arguments */
  if(!process_command_line(argc, argv)) {
    printf("example: drive -x 2 -y 2 \n");
    exit(1);
  }

  
  print_domain();
  print_problem();

}
