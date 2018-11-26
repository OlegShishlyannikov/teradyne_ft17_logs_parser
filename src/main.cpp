#include <fstream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <unistd.h>

#include "teradyne_parser.hpp"
#include "ft17_parser.hpp"

int main( int argc, char * argv[] )
{
  std::vector< std::string > teradyne_input_file_names;
  std::vector< std::string > ft17_input_file_names;
  std::string output_file_name;
  std::string version;
  
  char c;
  
  while(( c = getopt( argc, argv, "t:f:o:" )) != -1 ){

	switch( c ){

	case 't' : {
	  teradyne_input_file_names.push_back( std::string( optarg ));
	  break;
	}
	case 'o' : {
	  output_file_name = std::string( optarg );
	  break;
	}
	case 'f' : {
	  ft17_input_file_names.push_back( std::string( optarg ));
	  break;
	}
	}
  }

  if( ft17_input_file_names.size() == 0 ){ std::printf( "Error! Please specify input file names for ft17( -f ).\r\n" ); exit( 1 ); }
  if( teradyne_input_file_names.size() == 0 ){ std::printf( "Error! Please specify input file names for teradyne( -t ).\r\n" ); exit( 1 ); }
  if( output_file_name.length() == 0 ){ std::printf( "Error! Please specify output fie name.\r\n" ); exit( 1 ); }

  std::string teradyne_input_files_data;
  std::string ft17_input_files_data;

  for( std::string input_file_name : teradyne_input_file_names ){

  	std::ifstream input_files_stream( input_file_name.c_str() );
  	std::printf( "Open teradyne log file %s ...\r\n", input_file_name.c_str() );

  	if( input_files_stream.is_open() ){

  	  std::string line;

  	  while( std::getline( input_files_stream, line )){

  		teradyne_input_files_data += line;
  		teradyne_input_files_data += "\r\n";
		
  	  }
  	}
	
  	input_files_stream.close();
  }

  for( std::string input_file_name : ft17_input_file_names ){

	std::ifstream input_files_stream( input_file_name.c_str() );
	std::printf( "Open ft17 log file %s ...\r\n", input_file_name.c_str() );

	if( input_files_stream.is_open() ){

	  std::string line;

	  while( std::getline( input_files_stream, line )){

		ft17_input_files_data += line;
		ft17_input_files_data += "\r\n";
		
	  }
	}
	
	input_files_stream.close();
  }

  /* Parse logs & print report */
  teradyne_parser * _teradyne_parser = new teradyne_parser( teradyne_input_files_data );
  ft17_parser * _ft17_parser = new ft17_parser( ft17_input_files_data );
  _teradyne_parser->print_report( *_teradyne_parser->get_metadata() );
  _ft17_parser->print_report( *_ft17_parser->get_metadata() );

  std::vector< std::string > table;  
  std::vector< std::string > before_ett_table;
  std::vector< std::string > after_ett_table;
  std::vector< std::string > plus_table;
  std::vector< std::string > minus_table;  
  std::string table_header( "Lot;JobName;TeradyneTest#;FT17Test#;TeradynePin;FT17Pin;TeradyneMeasured;Unit;FT17Measured;Unit;Diff;Unit\r\n" );
  
  for( unsigned int i = 0; i < _teradyne_parser->get_metadata()->size(); i ++ ){

	for( unsigned int j = 0; j < _ft17_parser->get_metadata()->size(); j ++ ){

	  if(( std::get< 0 >( _teradyne_parser->get_metadata()->at( i )) == std::get< 0 >( _ft17_parser->get_metadata()->at( j ))) &&
		 std::get< 1 >( _teradyne_parser->get_metadata()->at( i )) == std::get< 1 >( _ft17_parser->get_metadata()->at( j ))){

		static int lot;
		std::string job_name = std::get< 1 >( _teradyne_parser->get_metadata()->at( i ));
		std::pair< std::map< std::string, std::string >, std::map< int, std::map< std::string, std::string >>> * teradyne_lot_metadata = &( std::get< 2 >( _teradyne_parser->get_metadata()->at( i )));
		std::pair< std::map< std::string, std::string >, std::map< int, std::map< std::string, std::string >>> * ft17_lot_metadata = &( std::get< 2 >( _ft17_parser->get_metadata()->at( j )));
		std::map< std::string, std::string > * teradyne_lot_header = &( teradyne_lot_metadata->first );
		std::map< std::string, std::string > * ft17_lot_header = &( ft17_lot_metadata->first );
		std::map< int, std::map< std::string, std::string >> * teradyne_lot_tests_meta = &( teradyne_lot_metadata->second );
		std::map< int, std::map< std::string, std::string >> * ft17_lot_tests_meta = &( ft17_lot_metadata->second );
		
		if( lot !=  std::get< 0 >( _teradyne_parser->get_metadata()->at( i ))){

		  lot = std::get< 0 >( _teradyne_parser->get_metadata()->at( i ));
		  table.push_back( "\r\n" );
		  table.push_back( table_header );
		  table.push_back( "\r\n" );
		  table.push_back( std::to_string( lot ) + ";" + job_name + ";;;;;;;;;;\r\n" );
		  table.push_back( "\r\n" );

		  before_ett_table.push_back( "\r\n" );
		  before_ett_table.push_back( table_header );
		  before_ett_table.push_back( "\r\n" );
		  before_ett_table.push_back( std::to_string( lot ) + ";" + job_name + ";;;;;;;;;;\r\n" );
		  before_ett_table.push_back( "\r\n" );

		  after_ett_table.push_back( "\r\n" );
		  after_ett_table.push_back( table_header );
		  after_ett_table.push_back( "\r\n" );
		  after_ett_table.push_back( std::to_string( lot ) + ";" + job_name + ";;;;;;;;;;\r\n" );
		  after_ett_table.push_back( "\r\n" );

		  plus_table.push_back( "\r\n" );
		  plus_table.push_back( table_header );
		  plus_table.push_back( "\r\n" );
		  plus_table.push_back( std::to_string( lot ) + ";" + job_name + ";;;;;;;;;;\r\n" );
		  plus_table.push_back( "\r\n" );

		  minus_table.push_back( "\r\n" );
		  minus_table.push_back( table_header );
		  minus_table.push_back( "\r\n" );
		  minus_table.push_back( std::to_string( lot ) + ";" + job_name + ";;;;;;;;;;\r\n" );
		  minus_table.push_back( "\r\n" );

		}
		
		for( auto teradyne_lot_tests_meta_it = teradyne_lot_tests_meta->begin(); teradyne_lot_tests_meta_it != teradyne_lot_tests_meta->end(); teradyne_lot_tests_meta_it ++ ){

		  for( auto ft17_lot_tests_meta_it = ft17_lot_tests_meta->begin(); ft17_lot_tests_meta_it != ft17_lot_tests_meta->end(); ft17_lot_tests_meta_it ++ ){

			std::string teradyne_test_name = teradyne_lot_tests_meta_it->second.at( "test_name" );
			std::string teradyne_test_pin = teradyne_lot_tests_meta_it->second.at( "test_pin" );
			std::string ft17_test_name = ft17_lot_tests_meta_it->second.at( "test_name" );
			std::string ft17_test_pin = ft17_lot_tests_meta_it->second.at( "test_pin" );
			
			std::transform( teradyne_test_name.begin(), teradyne_test_name.end(), teradyne_test_name.begin(), ::toupper );
			std::transform( teradyne_test_pin.begin(), teradyne_test_pin.end(), teradyne_test_pin.begin(), ::toupper );
			std::transform( ft17_test_name.begin(), ft17_test_name.end(), ft17_test_name.begin(), ::toupper );
			std::transform( ft17_test_pin.begin(), ft17_test_pin.end(), ft17_test_pin.begin(), ::toupper );
			
			if(( teradyne_test_name == ft17_test_name ) &&
			   ( teradyne_test_pin == ft17_test_pin )){

			  std::map< std::string, int > units = {
				{ "pA", -12 },
				{ "nA", -9 },
				{ "uA", -6 },
				{ "mA", -3 },
				{ "A", 0 },
				{ "pV", -12 },
				{ "nV", -9 },
				{ "uV", -6 },
				{ "mV", -3 },
				{ "V", 0 },
				{ "пА", -12 },
				{ "нА", -9 },
				{ "мкА", -6 },
				{ "мА", -3 },
				{ "А", 0 },
				{ "пВ", -12 },
				{ "нВ", -9 },
				{ "мкВ", -6 },
				{ "мВ", -3 },
				{ "В", 0 }
			  };
			  
			  std::string lot_str = std::to_string( lot );
			  std::string teradyne_test_number = std::to_string( teradyne_lot_tests_meta_it->first );
			  std::string ft17_test_number = std::to_string( ft17_lot_tests_meta_it->first );
			  double teradyne_measured = std::atof( QString::fromStdString( teradyne_lot_tests_meta_it->second.at( "test_measured" )).split( " " )[ 0 ].toStdString().c_str() );
			  std::string teradyne_unit = QString::fromStdString( teradyne_lot_tests_meta_it->second.at( "test_measured" )).split( " " )[ 1 ].toStdString();
			  double ft17_measured = std::atof( ft17_lot_tests_meta_it->second.at( "test_measured" ).c_str() );
			  std::string ft17_unit = ft17_lot_tests_meta_it->second.at( "test_measurement_unit" );
			  std::string diff_unit = teradyne_unit;
			  
			  if(( units.find( teradyne_unit ) != units.end() ) && ( units.find( ft17_unit ) != units.end())){

				teradyne_measured *= std::pow( 10.0f, units.at( teradyne_unit ) - units.at( ft17_unit ));
				teradyne_unit = ft17_unit;
				diff_unit = teradyne_unit;
				
			  }

			  double diff = teradyne_measured - ft17_measured;
			  
			  std::string table_str = ";;" + teradyne_test_number + ";" + ft17_test_number + ";" + teradyne_test_pin + ";" + ft17_test_pin + ";" + std::to_string( teradyne_measured ) + ";" +
				teradyne_unit + ";" + std::to_string( ft17_measured ) + ";" + ft17_unit + ";" +	std::to_string( diff ) + ";" + diff_unit + "\r\n";

			  if( job_name == "BEFORE_ETT" ) before_ett_table.push_back( table_str );
			  else if( job_name == "AFTER_ETT" ) after_ett_table.push_back( table_str );
			  else if( job_name == "PLUS" ) plus_table.push_back( table_str );
			  else if( job_name == "MINUS" ) minus_table.push_back( table_str );
			  table.push_back( table_str );
			  
			}
		  }
		}
	  }
	}
  }

  // std::printf( "Table :\r\n" );

  // for( unsigned int i = 0; i < table.size(); i ++ ) std::printf( "%s", table[ i ].c_str() );

  // std::printf( "Before ETT table :\r\n" );

  // for( unsigned int i = 0; i < before_ett_table.size(); i ++ ) std::printf( "%s", before_ett_table[ i ].c_str() );

  // std::printf( "After ETT table :\r\n" );

  // for( unsigned int i = 0; i < after_ett_table.size(); i ++ ) std::printf( "%s", after_ett_table[ i ].c_str() );

  // std::printf( "Plus table :\r\n" );

  // for( unsigned int i = 0; i < plus_table.size(); i ++ ) std::printf( "%s", plus_table[ i ].c_str() );

  // std::printf( "Minus table :\r\n" );

  // for( unsigned int i = 0; i < minus_table.size(); i ++ ) std::printf( "%s", minus_table[ i ].c_str() );

  std::ofstream output_stream( output_file_name.c_str() );

  if( output_stream.is_open() ){

	std::printf( "Writing data to %s\r\n", output_file_name.c_str() );
	for( unsigned int i = 0; i < table.size(); i ++ ) output_stream << table[ i ];

  } else std::printf( "Failed to open \"%s\" file\r\n", output_file_name.c_str() );
  
  output_stream.close();
  
  delete _teradyne_parser;
  delete _ft17_parser;
  return 0;
}
