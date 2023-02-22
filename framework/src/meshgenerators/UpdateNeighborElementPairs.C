//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// STL includes - add stuff here from CSVReader and/or DelimitedFileReader
//#include <fstream>

// MOOSE includes
//#include "CSVReader.h"
//#include "MooseUtils.h"
#include <fstream>
#include "UpdateNeighborElementPairs.h"
#include "InputParameters.h"
#include "MooseUtils.h"
#include <iostream>

registerMooseObject("MooseApp", UpdateNeighborElementPairs);

InputParameters
UpdateNeighborElementPairs::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addClassDescription("This is a hard-code adding of a pair.");
  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addParam<std::vector<BoundaryName>>("adj_sidesets",
                                                     "The adjacent sidesets of the interfaces");
  params.addParam<std::vector<BoundaryName>>("sep_sidesets",
                                                     "The separated sidesets of the interfaces");
  params.addRequiredParam<FileName>("csv_file",
                                    "The name of the CSV file to read. Currently, with "
                                    "the exception of the header row, only numeric "
                                    "values are supported.");
  params.addParam<bool>("header",
                        "When true it is assumed that the first row contains column headers, these "
                        "headers are used as the VectorPostprocessor vector names. If false the "
                        "file is assumed to contain only numbers and the vectors are named "
                        "automatically based on the column number (e.g., 'column_0000', "
                        "'column_0001'). If not supplied the reader attempts to auto detect the "
                        "headers.");
  params.addParam<std::string>("delimiter",
                               "The column delimiter. Despite the name this can read files "
                               "separated by delimiter other than a comma. If this options is "
                               "omitted it will read comma or space separated files.");
  params.addParam<bool>(
      "ignore_empty_lines", true, "When true new empty lines in the file are ignored.");                       
                        
  return params;
}

UpdateNeighborElementPairs::UpdateNeighborElementPairs(const InputParameters & parameters)
  : MeshGenerator(parameters),    
    _input(getMesh("input")),
    _adj_sidesets(getParam<std::vector<BoundaryName>>("adj_sidesets")),
    _sep_sidesets(getParam<std::vector<BoundaryName>>("sep_sidesets"))
     
{  
}

std::unique_ptr<MeshBase>
UpdateNeighborElementPairs::generate()
{

  /// The MOOSE delimited file reader.
  MooseUtils::DelimitedFileReader csv_reader(getParam<FileName>("csv_file"), &_communicator);
  csv_reader.setIgnoreEmptyLines(getParam<bool>("ignore_empty_lines"));
  if (isParamValid("header"))
    csv_reader.setHeaderFlag(getParam<bool>("header")
                                 ? MooseUtils::DelimitedFileReader::HeaderFlag::ON
                                 : MooseUtils::DelimitedFileReader::HeaderFlag::OFF);
  if (isParamValid("delimiter"))
    csv_reader.setDelimiter(getParam<std::string>("delimiter"));

  csv_reader.read();
  //const std::vector<std::string> & csv_names = csv_reader.getNames(); 
  const std::vector<std::vector<double>> & csv_data = csv_reader.getData();



  std::unique_ptr<MeshBase> mesh = std::move(_input);
  

  BoundaryInfo & boundary_info = mesh->get_boundary_info();
  

  // set all adjacent interfaces as truly adjacent
  auto elem_sideset_ids = MooseMeshUtils::getBoundaryIDs(*mesh, _adj_sidesets, true);
  std::set<boundary_id_type> sidesets_e(elem_sideset_ids.begin(), elem_sideset_ids.end());
   
  for (auto & bnd_id : sidesets_e)
  {
    boundary_info.sideset_adjacency(bnd_id) = true;
  }

  // set all separated interfaces as falsely adjacent
  auto neigh_sideset_ids = MooseMeshUtils::getBoundaryIDs(*mesh, _sep_sidesets, true);
  std::set<boundary_id_type> sidesets_n(neigh_sideset_ids.begin(), neigh_sideset_ids.end());
  
  for (auto & bnd_id : sidesets_n)
  {
    boundary_info.sideset_adjacency(bnd_id) = false;
  }


  // Read a CSV file that contains 4 columns [elem_id, neighbor_id, elem_side, neighbor_side]
  // That is the arrangement provided by the SurfacesI array from DEIP, and it has the same
  // convention as MOOSE that the elem side is the lower block ID than the neighbor side.
  // The file must have EVERY e-n pair for ALL those sidesets listed. The set_neighbor function
  // doesn't care at all about boundary id, so it doesn't matter what order these pairs are.
  // And the sidesets already have which elements belong to them. So we just need all the pairs
  // loaded back in.
  
  // TODO: add error checking for the element IDs and Side IDs
  //       comparing to the maximum number of side sets in the domain
  
  for (long unsigned int i = 0; i < csv_data[0].size(); i++) 
	{
		int elem_id =       csv_data[0][i];
		int neighbor_id =   csv_data[1][i];
		int elem_side =     csv_data[2][i];
		int neighbor_side = csv_data[3][i];  
	 	std::cout << elem_id << " " << neighbor_id << " " << elem_side << " " << neighbor_side;
	 	std::cout << std::endl;
		Elem * current_elem = mesh->elem_ptr(elem_id);
		Elem * neighbor_elem = mesh->elem_ptr(neighbor_id);
		current_elem->set_neighbor(elem_side, neighbor_elem);
		neighbor_elem->set_neighbor(neighbor_side, current_elem);	 	
	}
  Partitioner::set_node_processor_ids(*mesh);
  return dynamic_pointer_cast<MeshBase>(mesh);
}

