# UpdateNeighborElementPairs
!syntax description /Mesh/UpdateNeighborElementPairs

The UpdateNeighborElementPairs class is a subclass of the MeshGenerator class in the MOOSE framework. This class is responsible for updating the adjacency of element pairs on a mesh.
#TODO from recording of meeting connection to breakmeshbyblock
## Description
This object is used to update the pair of neighbor elements in a mesh. The mesh to be modified is specified using the input parameter, while the pairs are specified using the csv_file parameter. The `UpdateNeighborElementPairs` class reads a CSV file containing numeric values and updates the adjacency of element pairs on a mesh. The class first sets all adjacent interfaces as truly adjacent. Then, it sets all separated interfaces as truly separated. The adjacency is determined based on the information read from the CSV file. Finally, the updated mesh is returned.


## Inputs

The class requires the following inputs:

    input: The mesh we want to modify.
    adj_sidesets: The adjacent sidesets of the interfaces.
    sep_sidesets: The separated sidesets of the interfaces.
    csv_file: The name of the CSV file to read. The file contains numeric values only.
    header: A flag indicating whether the first row contains column headers. If this is true, these headers are used as the vector names. If false, the file is assumed to contain only numbers and the vectors are named automatically based on the column number.
    delimiter: The column delimiter. This can read files separated by delimiters other than a comma.
    ignore_empty_lines: A flag indicating whether new empty lines in the file are ignored.

## Member Functions

The class has the following member functions:

    validParams(): Returns the valid parameters for the UpdateNeighborElementPairs class.
    UpdateNeighborElementPairs(const InputParameters & parameters): The constructor of the class, takes in the input parameters.
    generate(): Generates the updated mesh.


## Example Input File Syntax
!listing /test/tests/meshgenerators/update_element_neighbor_pairs/czm_multiple_action_and_materials_mesh_in.i block=Mesh



















































