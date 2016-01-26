#include <iostream>
#include <fstream>
#include <sstream>
#include <cerrno>
#include <cstdio>
#include <ctime>
#if defined __GNUC__ && __GNUC__ >= 3 && __GNUC_MINOR__ > 0
#include <ext/stdio_filebuf.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#include "global.h"
#include "actions.h"
#include "client.h"
#include "domains.h"
#include "exceptions.h"
#include "planners.h"
#include "problems.h"
#include "states.h"

#ifndef CLIENT_NAME
#define  CLIENT_NAME	"bonet@ldc.usb.ve"
#endif

extern "C" char* strtok_r( char*, const char*, char** );
extern int yyparse();
extern FILE* yyin;
std::string current_file;
static unsigned combo = 0;

void
printBanner( std::ostream &os )
{
  os << "This is m(ini)GPT. Ver 1.0" << std::endl
     << "(Developed for IPC-4 by B. Bonet and H. Geffner upon a code from CMU.)"
     << std::endl;
}

void
printUsage( std::ostream &os )
{
  printBanner( os );
  os << std::endl
     << "Usage: planner <option>* <host>:<port> <problem-file> <problem-name>"
     << std::endl << std::endl
     << "Options:" << std::endl << std::endl
     << "  [-a]                      (toggle hash-all states, default = off)" << std::endl
     << "  [-c <cutoff>]             (default = <infty>)" << std::endl
     << "  [-d <dead-end-value>]     (default = <infty>)" << std::endl
     << "  [-e <epsilon>]            (default = 0.0)" << std::endl
     << "  [-h <heuristic-stack>]    (default = \"atom-min-1-forward|min-min-lrtp\")"
     << std::endl
     << "  [-i <initial-hash-size>]  (default = 16536)" << std::endl
     << "  [-m <max-database-size>]  (default = 32)" << std::endl
     << "  [-n <noise-level>]        (default = 0.0)" << std::endl
     << "  [-p <planner>]            (default = lrtdp)" << std::endl
     << "  [-r <random-seed>]        (default = 0)" << std::endl
     << "  [-s <simulations>]        (default = 0)" << std::endl
     << "  [-v <verbose-level>]      (default = 0)" << std::endl
     << "  [-w <heuristic-weight>]   (default = 1)" << std::endl
     << "  [-z]                      (toggle domain analysis, default = off)" << std::endl
     << std::endl
     << "  <planner>         := random | vi | lrtdp | asp | hdp-<n>" << std::endl
     << "  <heuristic-stack> := <heuristic-stack> '|' <heuristic> | <heuristic>" << std::endl
     << "  <heuristic>       := zero | ff | look-<n> | min-min-lrtdp |" << std::endl
     << "                       min-min-ida* | min-min-ida*-no-tt |" << std::endl
     << "                       atom-min-<n>-forward | atom-min-<n>-backward" << std::endl
     << std::endl
     << "Examples:"
     << std::endl << std::endl
     << "  1. Solve problem `p1' in file `ex.pddl' with the lrtdp algorithm and a plain"
     << std::endl
     << "     min-min-lrtdp heuristic:"
     << std::endl << std::endl
     << "     $ planner -h \"zero|min-min-lrtdp\" <host>:<port> ex.pddl p1"
     << std::endl << std::endl
     << "  2. Use the value iteration algorithm starting with the zero vector:"
     << std::endl << std::endl
     << "     $ planner -h zero -p vi <host>:<port> ex.pddl p1"
     << std::endl << std::endl
     << "  3. Use lrtdp with the FF heuristic:"
     << std::endl << std::endl
     << "     $ planner -h ff <host>:<port> ex.pddl p1"
     << std::endl << std::endl
     << "  4. Use lrtdp with min-min-ida* heuristic computed using atom-min-1-forward"
     << std::endl
     << "     heuristic:"
     << std::endl << std::endl
     << "     $ planner -h \"atom-min-1-forward|min-min-ida*\" <host>:<port> ex.pddl p1"
     << std::endl << std::endl
     << "  5. Use lrtdp with a two-step lookahead with min-min-lrtdp heuristic:"
     << std::endl << std::endl
     << "     $ planner -h \"atom-min-1-forward|min-min-lrtdp|look-2\" <host>:<port> ex.pddl p1"
     << std::endl << std::endl
     << "  6. Use lrtdp with a 2-subset-atom backward heuristic:"
     << std::endl << std::endl
     << "     $ planner -h atom-min-2-backward <host>:<port> ex.pddl p1"
     << std::endl << std::endl
     << "  7. Use asp with ff heuristic, 10 simulated trials and cutoff of 25:"
     << std::endl << std::endl
     << "     $ planner -p asp -s 10 -c 25 -h ff <host>:<port> ex.pddl p1"
     << std::endl << std::endl
     << "  8. Use hdp(0,2) with ff heuristic and epsilon = .001:"
     << std::endl << std::endl
     << "     $ planner -p hdp-2 -h ff -e .001 <host>:<port> ex.pddl p1"
     << std::endl << std::endl;
}

bool
readArguments( int argc, char **argv, char* &hostport, char* &file, char* &problem )
{
  if( argc == 1 ) goto usage;
  ++argv;
  --argc;
  while( argv[0][0] == '-' )
    switch( argv[0][1] )
      {
      case 'a':
	gpt::hash_all = (gpt::hash_all?false:true);
	++argv;
	--argc;
	break;
      case 'c':
	gpt::cutoff = atoi( argv[1] );
	argv += 2;
	argc -= 2;
	break;
      case 'd':
	gpt::dead_end_value = (unsigned)gpt::heuristic_weight * atoi( argv[1] );
	argv += 2;
	argc -= 2;
	break;
      case 'e':
	gpt::epsilon = atof( argv[1] );
	argv += 2;
	argc -= 2;
	break;
      case 'h':
	gpt::default_hp =
	  !strncmp(gpt::heuristic,argv[1],9) && (strlen(argv[1])==19) && !strcmp(&gpt::heuristic[10],&argv[1][10]);
	gpt::heuristic = argv[1];
	argv += 2;
	argc -= 2;
	break;
      case 'i':
	gpt::initial_hash_size = atoi( argv[1] );
	argv += 2;
	argc -= 2;
	break;
      case 'm':
	gpt::max_database_size = atoi( argv[1] );
	argv += 2;
	argc -= 2;
	break;
      case 'n':
	gpt::noise = true;
	gpt::noise_level = atof( argv[1] );
	argv += 2;
	argc -= 2;
	break;
      case 'p':
	gpt::default_hp = false;
	gpt::algorithm = argv[1];
	argv += 2;
	argc -= 2;
	break;
      case 'r':
	gpt::seed = atoi( argv[1] );
	argv += 2;
	argc -= 2;
	break;
      case 's':
	gpt::simulations = atoi( argv[1] );
	argv += 2;
	argc -= 2;
	break;
      case 'v':
	gpt::verbosity = atoi( argv[1] );
	argv += 2;
	argc -= 2;
	break;
      case 'w':
	gpt::heuristic_weight = atof( argv[1] );
	gpt::dead_end_value = gpt::dead_end_value * (unsigned)gpt::heuristic_weight;
	argv += 2;
	argc -= 2;
	break;
      case 'x':
	gpt::xtra = atoi( argv[1] );
	argv += 2;
	argc -= 2;
	break;
      case 'z':
	gpt::domain_analysis = (gpt::domain_analysis?false:true);
	++argv;
	--argc;
	break;
      default:
	goto usage;
      }

  if( argc != 3 )
    {
    usage:
      printUsage( std::cout );
      return( false );
    }
  else
    {
      hostport = argv[0];
      file = argv[1];
      problem = argv[2];
      return( true );
    }
}

static bool 
read_file( const char* name )
{
  yyin = fopen( name, "r" );
  if( yyin == NULL )
    {
      std::cout << "parser:" << name << ": " << strerror( errno ) << std::endl;
      return( false );
    }
  else
    {
      current_file = name;
      bool success;
      try
	{
	  success = (yyparse() == 0);
	}
      catch( Exception exception )
	{
	  fclose( yyin );
	  std::cout << exception << std::endl;
	  return( false );
	}
      fclose( yyin );
      return( success );
    }
}

static int 
connect( const char *hostname, int port )
{
  struct hostent *host = ::gethostbyname(hostname);
  if( !host )
    {
      perror( "gethostbyname" );
      return( -1 );
    }

  int sock = ::socket( PF_INET, SOCK_STREAM, 0 );
  if( sock == -1 )
    {
      perror( "socket" );
      return( -1 );
    }
  
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons( port );
  addr.sin_addr = *((struct in_addr *)host->h_addr);
  memset( &(addr.sin_zero), '\0', 8 );

  if( ::connect( sock, (struct sockaddr*)&addr, sizeof(addr) ) == -1 )
    {
      perror( "connect" );
      return( -1 );
  }
  return( sock );
}

static void
createHeuristics( const problem_t &problem, const char *heuristic )
{
  heuristic_t *heur;
  char *strstack = strdup( heuristic );
  char *lptr, *ptr = strtok_r( strstack, "| ", &lptr );
  while( ptr != NULL )
    {
      if( !strncasecmp( ptr, "look-", 5 ) )
	{
	  char *t = strdup( ptr );
	  char *lp, *p = strtok_r( t, "-", &lp );
	  p = strtok_r( NULL, "-", &lp );
	  int lookahead = atoi( p );
	  heur = new lookaheadHeuristic_t( problem, *gpt::hstack.top(), lookahead );
	  free( t );
	}
      else if( !strcasecmp( ptr, "zero" ) )
	{
	  heur = new zeroHeuristic_t( problem );
	}
      else if( !strcasecmp( ptr, "min-min-lrtdp" ) )
	{
	  heur = new minMinLRTDPHeuristic_t( problem, *gpt::hstack.top() );
	}
      else if( !strcasecmp( ptr, "min-min-ida*" ) )
	{
	  heur = new minMinIDAHeuristic_t( problem, *gpt::hstack.top(), true );
	}
      else if( !strcasecmp( ptr, "min-min-ida*-no-tt" ) )
	{
	  heur = new minMinIDAHeuristic_t( problem, *gpt::hstack.top(), false );
	}
      else if( !strncasecmp( ptr, "dijkstra-", 9 ) )
	{
	  char *t = strdup( ptr );
	  char *lp, *p = strtok_r( t, "-", &lp );
	  p = strtok_r( NULL, "-", &lp );
	  unsigned size = atoi( p );
	  heur = new dijkstraHeuristic_t( problem, *gpt::hstack.top(), size );
	  free( t );
	}
      else if( !strcasecmp( ptr, "min-min-a*" ) )
	{
	  throw Exception( "`min-min-a*' not yet implemented" );
	}
      else if( !strncasecmp( ptr, "atom-min-", 9 ) )
	{
	  char *t = strdup( ptr );
	  char *lp, *p = strtok_r( t, "-", &lp );
	  p = strtok_r( NULL, "-", &lp );
	  p = strtok_r( NULL, "-", &lp );
	  int m = atoi( p );
	  p = strtok_r( NULL, "-", &lp );
	  if( !p || (strcasecmp( p, "forward" ) && strcasecmp( p, "backward" )) )
	    {
	      free( t );
	      goto error;
	    }

	  if( !strcasecmp( p, "forward" ) )
	    {
	      if( m == 1 )
		heur = new atomMin1ForwardHeuristic_t( problem );
	      else
		heur = new atomMinMForwardHeuristic_t( problem, m );
	    }
	  else
	    {
	      atomMinMBackwardHeuristic_t *h = new atomMinMBackwardHeuristic_t( problem, m );
	      if( (h->number_databases() == 0) && gpt::default_hp )
		{
                  if( problem_t::number_atoms() < 32 )
                    {
                      combo = 1;
                      heur = new zeroHeuristic_t( problem );
		      std::cout << "<combo>: switching to combo-1" << std::endl;
                    }
                  else
                    {
                      combo = 2;
		      heur = new ffHeuristic_t( problem );
		      std::cout << "<combo>: switching to combo-2" << std::endl;
                    }
                  delete h;
		}
	      else
		heur = h;
	    }

	  free( t );
	}
      else if( !strcasecmp( ptr, "ff" ) )
	{
	  heur = new ffHeuristic_t( problem );
	}
      else
	{
	error:
	  std::ostringstream msg;
	  msg << "undefined heuristic `" << ptr << "'";
	  throw Exception( msg.str() );
	}
      gpt::hstack.push( heur );
      ptr = strtok_r( NULL, "| ", &lptr );
    }
  free( strstack );
}

static void
destroyHeuristics( void )
{
  while( !gpt::hstack.empty() )
    {
      delete gpt::hstack.top();
      gpt::hstack.pop();
    }
}

planner_t&
createPlanner( const problem_t &problem, const char *algorithm )
{
  planner_t *planner;
  if( combo == 1 )
    {
      algorithm = "lrtdp";
    }
  else if( combo == 2 )
    {
      algorithm = "asp";
    }

  if( !strcasecmp( algorithm, "asp" ) )
    {
      planner = new plannerASP_t( problem, *gpt::hstack.top(), gpt::simulations );
    }
  else if( !strcasecmp( algorithm, "random" ) )
    {
      planner = new plannerRANDOM_t( problem, *gpt::hstack.top() );
    }
  else if( !strcasecmp( algorithm, "vi" ) )
    {
      planner = new plannerVI_t( problem, *gpt::hstack.top(), gpt::epsilon );
    }
  else if( !strcasecmp( algorithm, "lrtdp" ) )
    {
      planner = new plannerLRTDP_t( problem, *gpt::hstack.top(), gpt::epsilon );
    }
  else if( !strncasecmp( algorithm, "hdp-", 4 ) )
    {
      char *t = strdup( algorithm );
      char *p = strtok( t, "-" );
      p = strtok( NULL, "-" );
      int j = atoi( p );
      planner = new plannerHDPZJ_t( problem, *gpt::hstack.top(), gpt::epsilon, j );
      free( t );
    }
  else
    {
      std::ostringstream msg;
      msg << "undefined planner `" << algorithm << "'";
      throw Exception( msg.str() );
    }
  return( *planner );
}

int 
main( int argc, char **argv )
{
  planner_t *planner;
  char *hostport, *file, *prob;
  ushort_t seed[3];

  // set command line
  std::ostringstream cmd;
  for( int i = 0; i < argc; ++i )
    cmd << argv[i] << " ";

  // read arguments and print banner
  if( !readArguments( argc, argv, hostport, file, prob ) ) return( -1 );
  printBanner( std::cout );
  std::cout << "**" << std::endl;
  std::cout << "COMMAND: " << cmd.str() << std::endl;
  std::cout << "PLANNER: \"" << gpt::algorithm << "\"" << std::endl;
  std::cout << "HEURISTIC: \"" << gpt::heuristic << "\"" << std::endl;
  std::cout << "**" << std::endl;

  // set random seeds
  seed[0] = seed[1] = seed[2] = gpt::seed;
  srand48( gpt::seed );
  seed48( seed );

  // parse input file
  if( !read_file( file ) )
    {
      std::cout << "<main>: ERROR: couldn't read problem file `" << file << "'" << std::endl;
      return( -1 );
    }

  // obtain problem from file
  problem_t *problem = (problem_t*)problem_t::find( prob );
  if( !problem )
    {
      std::cout << "<main>: ERROR: problem `" << prob << "' is not defined in file '"
		<< file << "'" << std::endl;
      return( -1 );
    }

  if( gpt::verbosity >= 300 )
    {
      std::cout << "<domain-begin>" << std::endl
		<< problem->domain() << std::endl
		<< "<domain-end>" << std::endl;
    }

  //xxxxxx move after timer is started
  // instantiate actions
  try
    {
      problem->instantiate_actions();
      problem->flatten();
      //problem->analyze_symmetries();
      state_t::initialize( *problem );

      if( gpt::verbosity >= 300 )
	{
	  std::cout << "<problem-begin>" << std::endl << "goal: ";
	  problem->goalT().print( std::cout );
	  std::cout << std::endl << "<problem-end>" << std::endl;
	}
    }
  catch( Exception exception )
    {
      std::cout << exception << std::endl;
      return( -1 );
    }

  if( gpt::verbosity >= 300 )
    std::cout << "**" << std::endl;
  std::cout << "<begin-session>" << std::endl;

  // bind to socket
  char *host = strtok( hostport, ":" );
  char *portstr = strtok( NULL , ":" );
  int port = atoi( portstr );
  int socket = connect( host, port );
  if( socket <= 0 )
    {
      std::cout << "<main>: ERROR: couldn't connect to host:port `"
		<< host << ":" << port << "'" << std::endl;
      return( -1 );
    }

  // create I/O streams bound to socket
  __gnu_cxx::stdio_filebuf<char> ofbuf( socket, std::ios_base::out, BUFSIZ );
  __gnu_cxx::stdio_filebuf<char> ifbuf( socket, std::ios_base::in, 1 );
#if defined __GNUC__ && __GNUC__ >= 3 && __GNUC_MINOR__ > 0
  //__gnu_cxx::stdio_filebuf<char> ofbuf( socket, std::ios_base::out, false, BUFSIZ );
  //__gnu_cxx::stdio_filebuf<char> ifbuf( socket, std::ios_base::in, false, 1 );
#else
  //std::filebuf ofbuf( socket );
  //std::filebuf ifbuf( socket );
#endif
  std::ostream os( &ofbuf );
  std::istream is( &ifbuf );

  // initialize algorithm + heuristic + planner and interact with server
  try
    {
      createHeuristics( *problem, gpt::heuristic );
      planner = &createPlanner( *problem, gpt::algorithm );
      problem->no_more_atoms();
      XMLClient_t client( planner, problem, CLIENT_NAME, is, os );
    }
  catch( Exception exception )
    {
      close( socket );
      std::cout << exception << std::endl;
      return( -1 );
    }

  // print statistics and clean
  planner->statistics( std::cout, gpt::verbosity );
  delete planner;
  destroyHeuristics();

  problem_t::unregister_use( problem );
  problem_t::clear();
  Domain::clear();
  state_t::statistics( std::cout );
  state_t::finalize();

  // return
#ifdef MEM_DEBUG
  std::cerr << "<end-session>" << std::endl;
#endif
  std::cout << "<end-session>" << std::endl;
  return( 0 );
}
