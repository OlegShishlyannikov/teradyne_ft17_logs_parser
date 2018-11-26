#include "teradyne_parser.hpp"

teradyne_parser::teradyne_parser( std::string & teradyne_raw_data )
{
  std::vector< std::tuple< int, std::string, std::pair< std::map< std::string, std::string >, std::map< int, std::map< std::string, std::string >>>, std::string, std::string >> teradyne_metadata;
  std::vector< std::string > teradyne_blocks = this->teradyne_get_blocks( teradyne_raw_data );
  
  for( std::string block : teradyne_blocks ){
	
	std::map< std::string, std::string > header_metadata;
	std::string header = this->teradyne_get_header( block );
	std::string body = this->teradyne_get_body( block );
	int lot = this->teradyne_parse_lot( header );
	std::string flow_id = this->teradyne_parse_flow_id( header );
	std::string job_name = this->teradyne_parse_job_name( header );

	if( flow_id == "ETT1" ) flow_id = "BEFORE_ETT";
	else if( flow_id == "ETT2" ) flow_id = "AFTER_ETT";
	else if( flow_id == "T_MINUS" ) flow_id = "MINUS";
	else if( flow_id == "T_PLUS" ) flow_id = "PLUS";
	  
	header_metadata.insert( std::make_pair( "date_time", this->teradyne_parse_datetime( header )));
	header_metadata.insert( std::make_pair( "prog_name", this->teradyne_parse_prog_name( header )));
	header_metadata.insert( std::make_pair( "job_name", job_name ));
	header_metadata.insert( std::make_pair( "lot", std::to_string( lot )));
	header_metadata.insert( std::make_pair( "operator", this->teradyne_parse_operator( header )));
	header_metadata.insert( std::make_pair( "test_mode", this->teradyne_parse_test_mode( header )));
	header_metadata.insert( std::make_pair( "node_name", this->teradyne_parse_node_name( header )));
	header_metadata.insert( std::make_pair( "part_type", this->teradyne_parse_part_type( header )));
	header_metadata.insert( std::make_pair( "channel_map", this->teradyne_parse_channel_map( header )));
	header_metadata.insert( std::make_pair( "environment", this->teradyne_parse_environment( header )));
	header_metadata.insert( std::make_pair( "family_id", this->teradyne_parse_family_id( header )));
	header_metadata.insert( std::make_pair( "floor_id", this->teradyne_parse_floor_id( header )));
	header_metadata.insert( std::make_pair( "flow_id", flow_id ));
	header_metadata.insert( std::make_pair( "test_temp", this->teradyne_parse_test_temp( header )));
	header_metadata.insert( std::make_pair( "user_text", this->teradyne_parse_user_text( header )));

	std::map< int, std::map< std::string, std::string >> tests_metadata = this->teradyne_get_tests( body );
	std::tuple< std::string, std::string > tests_result = this->teradyne_check_tests( tests_metadata );
	std::tuple< int, std::string, std::pair< std::map< std::string, std::string >, std::map< int, std::map< std::string, std::string >>>, std::string, std::string >
	  lot_meta = { lot, flow_id, std::make_pair( header_metadata, tests_metadata ), std::get< 0 >( tests_result ), std::get< 1 >( tests_result )};

	bool exists = false;
	int pos = 0;
	for( unsigned int i = 0; i < teradyne_metadata.size(); i ++ ){

	  if(( std::get< 0 >( lot_meta ) == std::get< 0 >( teradyne_metadata[ i ])) && ( std::get< 1 >( lot_meta ) == std::get< 1 >( teradyne_metadata[ i ]))){

		exists = true;
		pos = i;
		break;

	  } else {

		exists = false;
		pos = 0;
		continue;
		
	  }
	}

	if( exists ){

	  if( QDateTime::fromString( QString::fromStdString( std::get< 2 >( lot_meta ).first.at( "date_time" )), "MM/dd/yyyy HH:mm:ss" ).secsTo( QDateTime::fromString( QString::fromStdString( std::get< 2 >( teradyne_metadata[ pos ]).first.at( "date_time" )), "MM/dd/yyyy HH:mm:ss" )) <= 0 ){

		teradyne_metadata[ pos ] = lot_meta;

	  }
	  
	} else teradyne_metadata.push_back( lot_meta );
  }

  this->teradyne_metadata = teradyne_metadata;
}

std::vector< std::tuple< int, std::string, std::pair< std::map< std::string, std::string >, std::map< int, std::map< std::string, std::string >>>, std::string, std::string >> * teradyne_parser::get_metadata()
{
  return &this->teradyne_metadata;
}

void teradyne_parser::print_report( std::vector< std::tuple< int, std::string, std::pair< std::map< std::string, std::string >, std::map< int, std::map< std::string, std::string >>>, std::string, std::string >> & teradyne_metadata )
{
  int lots_count = teradyne_metadata.size();
  int lots_failed = 0;
  int lots_passed = 0;
  std::string failed_lots;
  
  for( unsigned int i = 0; i < lots_count; i ++ ) if( std::get< 3 >( teradyne_metadata[ i ]) == "FAIL" ){

	  if( failed_lots.length() ) failed_lots += " ";
	  failed_lots += std::to_string( std::get< 0 >( teradyne_metadata[ i ]));
	  lots_failed ++;

	} else lots_passed ++;
  
  std::printf( "Teradyne report : \r\n" );
  std::printf( "Passed : %i;\r\nFailed : %i ( %s );\r\nTotal : %i;\r\n\r\n", lots_passed, lots_failed, ( lots_failed ) ? failed_lots.c_str() : "None", lots_count );
}

void teradyne_parser::print_log( std::vector< std::tuple< int, std::string, std::pair< std::map< std::string, std::string >, std::map< int, std::map< std::string, std::string >>>, std::string, std::string >> & teradyne_metadata )
{
  for( unsigned int i = 0; i < teradyne_metadata.size(); i ++ ){

	std::printf( "Lot#%i, FlowID : %s, Result : %s, Reason : %s, Tests count : %lu\r\n Header : {"
				 "\r\n\tdate_time : %s,\r\n\tprog_name : %s,\r\n\tjob_name : %s\r\n\tlot : %s,\r\n\toperator : %s,\r\n\ttest_mode : %s,"
				 "\r\n\tnode_name : %s,\r\n\tpart_type : %s,\r\n\tchannel_map : %s,\r\n\tenvironment : %s,\r\n\tfamily_id : %s,"
				 "\r\n\tfloor_id : %s,\r\n\tflow_id : %s,\r\n\ttest_temp : %s,\r\n\tuser_text : %s\r\n}\r\n",
				 std::get< 0 >( teradyne_metadata[ i ]),
				 std::get< 1 >( teradyne_metadata[ i ]).c_str(),
				 std::get< 3 >( teradyne_metadata[ i ]).c_str(),
				 std::get< 4 >( teradyne_metadata[ i ]).c_str(),
				 std::get< 2 >( teradyne_metadata[ i ]).second.size(),
				 std::get< 2 >( teradyne_metadata[ i ]).first.at( "date_time" ).c_str(),
				 std::get< 2 >( teradyne_metadata[ i ]).first.at( "prog_name" ).c_str(),
				 std::get< 2 >( teradyne_metadata[ i ]).first.at( "job_name" ).c_str(),
				 std::get< 2 >( teradyne_metadata[ i ]).first.at( "lot" ).c_str(),
				 std::get< 2 >( teradyne_metadata[ i ]).first.at( "operator" ).c_str(),
				 std::get< 2 >( teradyne_metadata[ i ]).first.at( "test_mode" ).c_str(),
				 std::get< 2 >( teradyne_metadata[ i ]).first.at( "node_name" ).c_str(),
				 std::get< 2 >( teradyne_metadata[ i ]).first.at( "part_type" ).c_str(),
				 std::get< 2 >( teradyne_metadata[ i ]).first.at( "channel_map" ).c_str(),
				 std::get< 2 >( teradyne_metadata[ i ]).first.at( "environment" ).c_str(),
				 std::get< 2 >( teradyne_metadata[ i ]).first.at( "family_id" ).c_str(),
				 std::get< 2 >( teradyne_metadata[ i ]).first.at( "floor_id" ).c_str(),
				 std::get< 2 >( teradyne_metadata[ i ]).first.at( "flow_id" ).c_str(),
				 std::get< 2 >( teradyne_metadata[ i ]).first.at( "test_temp" ).c_str(),
				 std::get< 2 >( teradyne_metadata[ i ]).first.at( "user_text" ).c_str()
				 );

	for( std::map< int, std::map< std::string, std::string >>::iterator it = std::get< 2 >( teradyne_metadata[ i ]).second.begin();
		 it != std::get< 2 >( teradyne_metadata[ i ]).second.end(); it ++ ){

	  std::printf( "Test#%i : { Site : %s, Result : %s, Test Name : %s, Pin : %s, Channel : %s, Low : %s, Measured : %s, High : %s, Force : %s, Loc : %s }\r\n",
				   it->first,
				   it->second.at( "site" ).c_str(),
				   it->second.at( "result" ).c_str(),
				   it->second.at( "test_name" ).c_str(),
				   it->second.at( "test_pin" ).c_str(),
				   it->second.at( "channel" ).c_str(),
				   it->second.at( "test_minimum" ).c_str(),
				   it->second.at( "test_measured" ).c_str(),
				   it->second.at( "test_maximum" ).c_str(),
				   it->second.at( "force" ).c_str(),
				   it->second.at( "loc" ).c_str()
				   );
  
	}
  }
}

std::tuple< std::string, std::string > teradyne_parser::teradyne_check_tests( std::map< int, std::map< std::string, std::string >> & tests_meta )
{
  std::tuple< std::string, std::string > result;

  for( std::map< int, std::map< std::string, std::string >>::iterator it = tests_meta.begin(); it != tests_meta.end(); it ++ ){

	if( it->second.at( "result" ) == "PASS" ) result = { "PASS", "OK" };
	else if( it->second.at( "result" ) == "FAIL" ){

	  result = { "FAIL", it->second.at( "test_name" )};
	  break;
	  
	}
  }

  return result;
}

std::vector< std::string > teradyne_parser::teradyne_get_blocks( std::string & teradyne_raw_data )
{
  QString data = QString::fromStdString( teradyne_raw_data );
  std::vector< std::string > result;
  QRegularExpression block_regexp( "Datalog report[[:space:]]+[[:digit:]]+/[[:digit:]]+/[[:digit:]]+[[:space:]]+[[:digit:]]+:[[:digit:]]+:[[:digit:]]+[[:space:]]+"
									"Prog Name:[[:space:]]*[[:graph:]]*[[:space:]]+"
									"Job Name:[[:space:]]*[[:graph:]]*[[:space:]]+"
									"Lot:[[:space:]]*[[:graph:]]*[[:space:]]+"
									"Operator:[[:space:]]*[[:graph:]]*[[:space:]]+"
									"Test Mode:[[:space:]]*[[:graph:]]*[[:space:]]+"
									"Node Name:[[:space:]]*[[:graph:]]*[[:space:]]+"
									"Part Type:[[:space:]]*[[:graph:]]*[[:space:]]+"
									"Channel map:[[:space:]]*[[:graph:]]*[[:space:]]+"
									"Environment:[[:space:]]*[[:graph:]]*[[:space:]]+"
									"FamilyID:[[:space:]]*[[:graph:]]*[[:space:]]+"
									"FloorID:[[:space:]]*[[:graph:]]*[[:space:]]+"
									"FlowID:[[:space:]]*[[:graph:]]*[[:space:]]+"
									"TstTemp:[[:space:]]*[[:graph:]]*[[:space:]]+"
									"UserText:[[:space:]]*[[:graph:]]*[[:space:]]+"
									"Site Number:[[:space:]]*[[:graph:]]*[[:space:]]+"
									"Device#:[[:space:]]*[^=]*[=]+"
									);

  QRegularExpressionMatchIterator block_regexp_match_it = block_regexp.globalMatch( data );
  
  while( block_regexp_match_it.hasNext() ){

	QRegularExpressionMatch match = block_regexp_match_it.next();
	result.push_back( match.captured( 0 ).toStdString() );
	
  }
  
  return result;
}

std::string teradyne_parser::teradyne_get_header( std::string & teradyne_block )
{
  QString data = QString::fromStdString( teradyne_block );
  std::string result;
  QRegularExpression header_regexp( "Datalog report[[:space:]]+[[:digit:]]+/[[:digit:]]+/[[:digit:]]+[[:space:]]+[[:digit:]]+:[[:digit:]]+:[[:digit:]]+[[:space:]]+"
									"Prog Name:[[:space:]]*[[:graph:]]*[[:space:]]+"
									"Job Name:[[:space:]]*[[:graph:]]*[[:space:]]+"
									"Lot:[[:space:]]*[[:graph:]]*[[:space:]]+"
									"Operator:[[:space:]]*[[:graph:]]*[[:space:]]+"
									"Test Mode:[[:space:]]*[[:graph:]]*[[:space:]]+"
									"Node Name:[[:space:]]*[[:graph:]]*[[:space:]]+"
									"Part Type:[[:space:]]*[[:graph:]]*[[:space:]]+"
									"Channel map:[[:space:]]*[[:graph:]]*[[:space:]]+"
									"Environment:[[:space:]]*[[:graph:]]*[[:space:]]+"
									"FamilyID:[[:space:]]*[[:graph:]]*[[:space:]]+"
									"FloorID:[[:space:]]*[[:graph:]]*[[:space:]]+"
									"FlowID:[[:space:]]*[[:graph:]]*[[:space:]]+"
									"TstTemp:[[:space:]]*[[:graph:]]*[[:space:]]+"
									"UserText:[[:space:]]*[[:graph:]]*[[:space:]]+"
									"Site Number:[[:space:]]*[[:graph:]]*[[:space:]]+"
									"Device#:[[:space:]]*[[:graph:]]"
									);

  QRegularExpressionMatch header_regexp_match = header_regexp.match( data );

  if( header_regexp_match.hasMatch() ){

	result = header_regexp_match.captured( 0 ).toStdString();
	
  }
  
  return result;
}

std::string teradyne_parser::teradyne_get_body( std::string & teradyne_block )
{
  QString data = QString::fromStdString( teradyne_block );
  std::string result;
  QRegularExpression body_regexp( "Number[[:space:]]+Site[[:space:]]+Result[[:space:]]+Test Name[[:space:]]+Pin[[:space:]]+Channel[[:space:]]+Low[[:space:]]+Measured[[:space:]]+High[[:space:]]+Force[[:space:]]+Loc[[:space:]]+[^=]*[=]+" );
  QRegularExpressionMatch body_regexp_match = body_regexp.match( data );

  if( body_regexp_match.hasMatch() ){

	result = body_regexp_match.captured( 0 ).toStdString();
	
  }

  return result;
}

std::string teradyne_parser::teradyne_parse_datetime( std::string & teradyne_header )
{
  std::function< std::string( std::string &, std::regex & ) > parse_substring = []( std::string & to_parse, std::regex & re ) -> std::string {
	std::string data = to_parse;
	std::smatch match;
	std::string result;

	while( std::regex_search( data, match, re )){

	  for( auto matched : match ){

		result = matched.str();
		data = match.suffix().str();
	  
	  }
	}

	return result;
  };
	
  std::string result;
  std::string data = teradyne_header;
  std::smatch datetime_regex_match;
  std::regex datetime_regexp( "[\\d]+/[\\d]+/[\\d]+[\\s]+[\\d]+:[\\d]+:[\\d]+" );
  std::regex datetime_substr_regexp( "[\\d]+/[\\d]+/[\\d]+[\\s]+[\\d]+:[\\d]+:[\\d]+" );

  while( std::regex_search( data, datetime_regex_match, datetime_regexp )){

	for( auto matched : datetime_regex_match ){

	  result = matched.str();
	  data = datetime_regex_match.suffix().str();
	  
	}
  }

  result = parse_substring( result, datetime_substr_regexp );
  return result;
}

std::string teradyne_parser::teradyne_parse_prog_name( std::string & teradyne_header )
{
  std::function< std::string( std::string &, std::regex & ) > parse_substring = []( std::string & to_parse, std::regex & re ) -> std::string {
	std::string data = to_parse;
	std::smatch match;
	std::string result;

	while( std::regex_search( data, match, re )){

	  for( auto matched : match ){

		result = matched.str();
		data = match.suffix().str();
	  
	  }
	}
	
	std::stringstream stream( result );

	for( std::string str; stream >> str; ) result = str;

	return result;
  };

  std::string result;
  std::string data = teradyne_header;
  std::smatch prog_name_regex_match;
  std::regex prog_name_regexp( "Prog Name:[\\s]*[\\w.]*" );
  std::regex prog_name_substr_regexp( "[\\s]{4}[\\w.]*" );

  while( std::regex_search( data, prog_name_regex_match, prog_name_regexp )){

	for( auto matched : prog_name_regex_match ){

	  result = matched.str();
	  data = prog_name_regex_match.suffix().str();
	  
	}
  }

  result = parse_substring( result, prog_name_substr_regexp );
  return result;
}

std::string teradyne_parser::teradyne_parse_job_name( std::string & teradyne_header )
{
  std::function< std::string( std::string &, std::regex & ) > parse_substring = []( std::string & to_parse, std::regex & re ) -> std::string {
	std::string data = to_parse;
	std::smatch match;
	std::string result;

	while( std::regex_search( data, match, re )){

	  for( auto matched : match ){

		result = matched.str();
		data = match.suffix().str();
	  
	  }
	}
	
	std::stringstream stream( result );

	for( std::string str; stream >> str; ) result = str;

	return result;
  };

  std::string result;
  std::string data = teradyne_header;
  std::smatch job_name_regex_match;
  std::regex job_name_regexp( "Job Name:[\\s]*[\\w.]*" );
  std::regex job_name_substr_regexp( "[\\s]{4}[\\w.]*" );

  while( std::regex_search( data, job_name_regex_match, job_name_regexp )){

	for( auto matched : job_name_regex_match ){

	  result = matched.str();
	  data = job_name_regex_match.suffix().str();
	  
	}
  }

  result = parse_substring( result, job_name_substr_regexp );
  return result;
}

int teradyne_parser::teradyne_parse_lot( std::string & teradyne_header )
{
  std::function< std::string( std::string &, std::regex & ) > parse_substring = []( std::string & to_parse, std::regex & re ) -> std::string {
	std::string data = to_parse;
	std::smatch match;
	std::string result;
	
	while( std::regex_search( data, match, re )){

	  for( auto matched : match ){

		result = matched.str();
		data = match.suffix().str();
	  
	  }
	}
	
	std::stringstream stream( result );

	for( std::string str; stream >> str; ) result = str;

	return result;
  };

  int result;
  std::string result_str;
  std::string data = teradyne_header;
  std::smatch lot_regex_match;
  std::regex lot_regexp( "Lot:[\\s]*[\\w.]*" );
  std::regex lot_substr_regexp( "[\\s]{4}[\\w.]*" );
  
  while( std::regex_search( data, lot_regex_match, lot_regexp )){

	for( auto matched : lot_regex_match ){

	  result_str = matched.str();
	  data = lot_regex_match.suffix().str();
	  
	}
  }
  
  result = std::atoi( parse_substring( result_str, lot_substr_regexp ).c_str() );
  return result;
}

std::string teradyne_parser::teradyne_parse_operator( std::string & teradyne_header )
{
  std::function< std::string( std::string &, std::regex & ) > parse_substring = []( std::string & to_parse, std::regex & re ) -> std::string {
	std::string data = to_parse;
	std::smatch match;
	std::string result;

	while( std::regex_search( data, match, re )){

	  for( auto matched : match ){

		result = matched.str();
		data = match.suffix().str();
	  
	  }
	}
	
	std::stringstream stream( result );

	for( std::string str; stream >> str; ) result = str;

	return result;
  };

  std::string result;
  std::string data = teradyne_header;
  std::smatch operator_regex_match;
  std::regex operator_regexp( "Operator:[\\s]*[\\w.]*" );
  std::regex operator_substr_regexp( "[\\s]{4}[\\w.]*" );
  
  while( std::regex_search( data, operator_regex_match, operator_regexp )){

	for( auto matched : operator_regex_match ){

	  result = matched.str();
	  data = operator_regex_match.suffix().str();
	  
	}
  }

  result = parse_substring( result, operator_substr_regexp );
  return result;
}

std::string teradyne_parser::teradyne_parse_test_mode( std::string & teradyne_header )
{
  std::function< std::string( std::string &, std::regex & ) > parse_substring = []( std::string & to_parse, std::regex & re ) -> std::string {
	std::string data = to_parse;
	std::smatch match;
	std::string result;

	while( std::regex_search( data, match, re )){

	  for( auto matched : match ){

		result = matched.str();
		data = match.suffix().str();
	  
	  }
	}
	
	std::stringstream stream( result );

	for( std::string str; stream >> str; ) result = str;

	return result;
  };

  std::string result;
  std::string data = teradyne_header;
  std::smatch test_mode_regex_match;
  std::regex test_mode_regexp( "Test Mode:[\\s]*[\\w.]*" );
  std::regex test_mode_substr_regexp( "[\\s]{4}[\\w.]*" );
  
  while( std::regex_search( data, test_mode_regex_match, test_mode_regexp )){

	for( auto matched : test_mode_regex_match ){

	  result = matched.str();
	  data = test_mode_regex_match.suffix().str();
	  
	}
  }

  result = parse_substring( result, test_mode_substr_regexp );
  return result;
}

std::string teradyne_parser::teradyne_parse_node_name( std::string & teradyne_header )
{
  std::function< std::string( std::string &, std::regex & ) > parse_substring = []( std::string & to_parse, std::regex & re ) -> std::string {
	std::string data = to_parse;
	std::smatch match;
	std::string result;

	while( std::regex_search( data, match, re )){

	  for( auto matched : match ){

		result = matched.str();
		data = match.suffix().str();
	  
	  }
	}
	
	std::stringstream stream( result );

	for( std::string str; stream >> str; ) result = str;

	return result;
  };

  std::string result;
  std::string data = teradyne_header;
  std::smatch node_name_regex_match;
  std::regex node_name_regexp( "Node Name:[\\s]*[\\w.]*" );
  std::regex node_name_substr_regexp( "[\\s]{4}[\\w.]*" );
  
  while( std::regex_search( data, node_name_regex_match, node_name_regexp )){

	for( auto matched : node_name_regex_match ){

	  result = matched.str();
	  data = node_name_regex_match.suffix().str();
	  
	}
  }

  result = parse_substring( result, node_name_substr_regexp );
  return result;
}

std::string teradyne_parser::teradyne_parse_part_type( std::string & teradyne_header )
{
  std::function< std::string( std::string &, std::regex & ) > parse_substring = []( std::string & to_parse, std::regex & re ) -> std::string {
	std::string data = to_parse;
	std::smatch match;
	std::string result;

	while( std::regex_search( data, match, re )){

	  for( auto matched : match ){

		result = matched.str();
		data = match.suffix().str();
	  
	  }
	}
	
	std::stringstream stream( result );

	for( std::string str; stream >> str; ) result = str;

	return result;
  };

  std::string result;
  std::string data = teradyne_header;
  std::smatch part_type_regex_match;
  std::regex part_type_regexp( "Part Type:[\\s]*[\\w.]*" );
  std::regex part_type_substr_regexp( "[\\s]{4}[\\w.]*" );

  while( std::regex_search( data, part_type_regex_match, part_type_regexp )){

	for( auto matched : part_type_regex_match ){

	  result = matched.str();
	  data = part_type_regex_match.suffix().str();
	  
	}
  }
  
  result = parse_substring( result, part_type_substr_regexp );
  return result;
}

std::string teradyne_parser::teradyne_parse_channel_map( std::string & teradyne_header )
{
  std::function< std::string( std::string &, std::regex & ) > parse_substring = []( std::string & to_parse, std::regex & re ) -> std::string {
	std::string data = to_parse;
	std::smatch match;
	std::string result;

	while( std::regex_search( data, match, re )){

	  for( auto matched : match ){

		result = matched.str();
		data = match.suffix().str();
	  
	  }
	}
	
	std::stringstream stream( result );

	for( std::string str; stream >> str; ) result = str;

	return result;
  };

  std::string result;
  std::string data = teradyne_header;
  std::smatch channel_map_regex_match;
  std::regex channel_map_regexp( "Channel map:[\\s]*[\\w.]*" );
  std::regex channel_map_substr_regexp( "[\\s]{4}[\\w.]*" );

  while( std::regex_search( data, channel_map_regex_match, channel_map_regexp )){

	for( auto matched : channel_map_regex_match ){

	  result = matched.str();
	  data = channel_map_regex_match.suffix().str();
	  
	}
  }
  
  result = parse_substring( result, channel_map_substr_regexp );
  return result;
}

std::string teradyne_parser::teradyne_parse_environment( std::string & teradyne_header )
{
  std::function< std::string( std::string &, std::regex & ) > parse_substring = []( std::string & to_parse, std::regex & re ) -> std::string {
	std::string data = to_parse;
	std::smatch match;
	std::string result;

	while( std::regex_search( data, match, re )){

	  for( auto matched : match ){

		result = matched.str();
		data = match.suffix().str();
	  
	  }
	}
	
	std::stringstream stream( result );

	for( std::string str; stream >> str; ) result = str;

	return result;
  };

  std::string result;
  std::string data = teradyne_header;
  std::smatch environment_regex_match;
  std::regex environment_regexp( "Environment:[\\s]*[\\w.]*" );
  std::regex environment_substr_regexp( "[\\s]{4}[\\w.]*" );
  
  while( std::regex_search( data, environment_regex_match, environment_regexp )){

	for( auto matched : environment_regex_match ){

	  result = matched.str();
	  data = environment_regex_match.suffix().str();
	  
	}
  }

  result = parse_substring( result, environment_substr_regexp );
  return result;
}

std::string teradyne_parser::teradyne_parse_family_id( std::string & teradyne_header )
{
  std::function< std::string( std::string &, std::regex & ) > parse_substring = []( std::string & to_parse, std::regex & re ) -> std::string {
	std::string data = to_parse;
	std::smatch match;
	std::string result;

	while( std::regex_search( data, match, re )){

	  for( auto matched : match ){

		result = matched.str();
		data = match.suffix().str();
	  
	  }
	}
	
	std::stringstream stream( result );

	for( std::string str; stream >> str; ) result = str;

	return result;
  };

  std::string result;
  std::string data = teradyne_header;
  std::smatch family_id_regex_match;
  std::regex family_id_regexp( "FamilyID:[\\s]*[\\w.]*" );
  std::regex family_id_substr_regexp( "[\\s]{4}[\\w.]*" );

  while( std::regex_search( data, family_id_regex_match, family_id_regexp )){

	for( auto matched : family_id_regex_match ){

	  result = matched.str();
	  data = family_id_regex_match.suffix().str();
	  
	}
  }

  result = parse_substring( result, family_id_substr_regexp );
  return result;
}

std::string teradyne_parser::teradyne_parse_floor_id( std::string & teradyne_header )
{
  std::function< std::string( std::string &, std::regex & ) > parse_substring = []( std::string & to_parse, std::regex & re ) -> std::string {
	std::string data = to_parse;
	std::smatch match;
	std::string result;

	while( std::regex_search( data, match, re )){

	  for( auto matched : match ){

		result = matched.str();
		data = match.suffix().str();
	  
	  }
	}
	
	std::stringstream stream( result );

	for( std::string str; stream >> str; ) result = str;

	return result;
  };

  std::string result;
  std::string data = teradyne_header;
  std::smatch floor_id_regex_match;
  std::regex floor_id_regexp( "FloorID:[\\s]*[\\w.]*" );
  std::regex floor_id_substr_regexp( "[\\s]{4}[\\w.]*" );

  while( std::regex_search( data, floor_id_regex_match, floor_id_regexp )){

	for( auto matched : floor_id_regex_match ){

	  result = matched.str();
	  data = floor_id_regex_match.suffix().str();
	  
	}
  }

  result = parse_substring( result, floor_id_substr_regexp );
  return result;
}

std::string teradyne_parser::teradyne_parse_flow_id( std::string & teradyne_header )
{
  std::function< std::string( std::string &, std::regex & ) > parse_substring = []( std::string & to_parse, std::regex & re ) -> std::string {
	std::string data = to_parse;
	std::smatch match;
	std::string result;

	while( std::regex_search( data, match, re )){

	  for( auto matched : match ){

		result = matched.str();
		data = match.suffix().str();
	  
	  }
	}
	
	std::stringstream stream( result );

	for( std::string str; stream >> str; ) result = str;

	return result;
  };

  std::string result;
  std::string data = teradyne_header;
  std::smatch flow_id_regex_match;
  std::regex flow_id_regexp( "FlowID:[\\s]*[\\w.]*" );
  std::regex flow_id_substr_regexp( "[\\s]{4}[\\w.]*" );

  while( std::regex_search( data, flow_id_regex_match, flow_id_regexp )){

	for( auto matched : flow_id_regex_match ){

	  result = matched.str();
	  data = flow_id_regex_match.suffix().str();
	  
	}
  }
  
  result = parse_substring( result, flow_id_substr_regexp );
  return result;
}

std::string teradyne_parser::teradyne_parse_test_temp( std::string & teradyne_header )
{
  std::function< std::string( std::string &, std::regex & ) > parse_substring = []( std::string & to_parse, std::regex & re ) -> std::string {
	std::string data = to_parse;
	std::smatch match;
	std::string result;

	while( std::regex_search( data, match, re )){

	  for( auto matched : match ){

		result = matched.str();
		data = match.suffix().str();
	  
	  }
	}
	
	std::stringstream stream( result );

	for( std::string str; stream >> str; ) result = str;

	return result;
  };

  std::string result;
  std::string data = teradyne_header;
  std::smatch test_temp_regex_match;
  std::regex test_temp_regexp( "TstTemp:[\\s]*[\\w.]*" );
  std::regex test_temp_substr_regexp( "[\\s]{4}[\\w.]*" );

  while( std::regex_search( data, test_temp_regex_match, test_temp_regexp )){

	for( auto matched : test_temp_regex_match ){

	  result = matched.str();
	  data = test_temp_regex_match.suffix().str();
	  
	}
  }
  
  result = parse_substring( result, test_temp_substr_regexp );
  return result;
}

std::string teradyne_parser::teradyne_parse_user_text( std::string & teradyne_header )
{
  std::function< std::string( std::string &, std::regex & ) > parse_substring = []( std::string & to_parse, std::regex & re ) -> std::string {
	std::string data = to_parse;
	std::smatch match;
	std::string result;

	while( std::regex_search( data, match, re )){

	  for( auto matched : match ){

		result = matched.str();
		data = match.suffix().str();
	  
	  }
	}
	
	std::stringstream stream( result );

	for( std::string str; stream >> str; ) result = str;

	return result;
  };

  std::string result;
  std::string data = teradyne_header;
  std::smatch user_text_regex_match;
  std::regex user_text_regexp( "UserText:[\\s]*[\\w.]*" );
  std::regex user_text_substr_regexp( "[\\s]{4}[\\w.]*" );

  while( std::regex_search( data, user_text_regex_match, user_text_regexp )){

	for( auto matched : user_text_regex_match ){

	  result = matched.str();
	  data = user_text_regex_match.suffix().str();
	  
	}
  }

  result = parse_substring( result, user_text_substr_regexp );
  return result;
}

std::map< int, std::map< std::string, std::string >> teradyne_parser::teradyne_get_tests( std::string & teradyne_body )
{
  std::map< int, std::map< std::string, std::string >> result;
  std::vector< std::string > tests;
  QString data = QString::fromStdString( teradyne_body );
  QRegularExpression test_regexp( "[[:graph:]]+[ ]{2,}[[:graph:]]+[ ]{2,}[[:word:]]{4}[ ]{2,}[[:graph:]]+[ ]{2,}[[:graph:]]+[ ]{2,}[[:graph:]]+[ ]{2,}[[:graph:]]+[ ]?[[:word:]]+[ ]{2,}[[:graph:]]+[ ]?[[:word:]]+[ ]{2,}[[:graph:]]+[ ]?[[:word:]]+[ ]{2,}[[:graph:]]+[ ]?[[:word:]]+[ ]{2,}[[:digit:]]" );
  QRegularExpressionMatchIterator test_regexp_it = test_regexp.globalMatch( data );

  while( test_regexp_it.hasNext() ){

	QRegularExpressionMatch match = test_regexp_it.next();
	tests.push_back( match.captured( 0 ).toStdString() );
	
  }

  for( std::string test : tests ){

	std::map< std::string, std::string > test_meta;
	std::string test_header( "number  site  result  test_name  test_pin  channel  test_minimum  test_measured  test_maximum  force  loc" );
	QStringList test_header_list = QString::fromStdString( test_header ).split( QRegularExpression( "[[:space:]]{2,}" ));
	QStringList test_list = QString::fromStdString( test ).split( QRegularExpression( "[[:space:]]{2,}" ));
	
	QString test_number = test_list[ 0 ];

	for( unsigned int i = 1; i < test_header_list.size(); i ++ ){

	  test_meta.insert( std::make_pair( test_header_list[ i ].toStdString(), test_list[ i ].toStdString() ));
	  
	}
	
	result.insert( std::make_pair( test_number.toInt(), test_meta ));
  }
  
  return result;
}

