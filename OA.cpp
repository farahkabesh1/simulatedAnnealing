#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <random>

using namespace std;

/*
components is 2D vector (vector of vectors) 
Inner vector represents net and each component index in that net 
Outer vector stores all the nets
*/

void readFile(const string& file, int& componentsNum, int& netsNum, int& rowsNum, int& colsNum, vector<vector<int> >& components) {
    
    ifstream inputFile(file);

    string line;
    getline(inputFile, line);                               //reads 1st line in txt file
    istringstream Iss(line);

    Iss >> componentsNum >> netsNum >> rowsNum >> colsNum;   //gets no. of components, nets, rows, cols from 1st line of input file

    components.resize(netsNum);                              //resizes components vector to have a size of number of nets

    for (int i = 0; i < netsNum; i++) {
        getline(inputFile, line);                            //reads remaining lines in txt file
        istringstream Iss2(line);

        int netfileComponents;
        Iss2 >> netfileComponents;                          //gets net information
        components[i].resize(netfileComponents);            //allocates space to store component indices for each net

        for (int j = 0; j < netfileComponents; j++) {       //get components for each net
            int component;
            Iss2 >> component;
            components[i][j] = component;                   //assigns component indices for each net
        }
    }

    inputFile.close();
}

void initialPlacement(int rowsNum, int colsNum, const vector<vector<int> >& components, vector<vector<int> >& grid, vector<vector<int> >& savedpos) {
    
    grid.assign(rowsNum, vector<int>(colsNum, -1));          //initialize all elements to -1 in grid 2D vector (empty place)
    // random_device rd;
    mt19937 gen(2);
    int value = 0;
    int row, col;
                                                                                  
    for (int i = 0; i < components.size(); i++) { 
        for (int j = 0; j < components[i].size(); j++) {
             value = components[i][j];             

            uniform_int_distribution<int> rowDist(0, rowsNum - 1);
            uniform_int_distribution<int> colDist(0, colsNum - 1);

            //check that the value is not already placed
            if (savedpos[value][0] == -1 && savedpos[value][1] == -1) {
                do {
                    row = rowDist(gen);
                    col = colDist(gen);
                } while (grid[row][col] != -1);   

                grid[row][col] = value;     

                // savedpos[index=ComponentNo] = {row,col}
                savedpos[value][0] = row;
                savedpos[value][1] = col;
            }    
        }
    }
}

void printPlacement(const vector<vector<int> >& grid) {
    
    for (int i = 0; i < grid.size(); i++) {
        for (int j = 0; j < grid[i].size(); j++) {
            
            int component = grid[i][j];
            
            if (component == -1) {
                cout <<setw(3)<< '-' << " ";
            } else {
                cout <<setw(3)<< component << " ";
            }
        }
        cout << endl;
    }
}

int calculateWireLength(const vector<vector<int> >& savedpos, const vector<vector<int> >& components) {
    int wireLength = 0;
    int i = 0;

    for(int k=0; k < components.size(); k++){

        int minY = INT_MAX;                       
        int maxY = INT_MIN;                             
        int minX = INT_MAX;                       
        int maxX = INT_MIN; 

        for (int j = 0; j < components[k].size(); j++) {
            i = components[k][j];
            if (savedpos[i][0] < minY) minY = savedpos[i][0];
            if (savedpos[i][0] > maxY) maxY = savedpos[i][0]; 
            if (savedpos[i][1] < minX) minX = savedpos[i][1];
            if (savedpos[i][1] > maxX) maxX = savedpos[i][1];
        }
            
        wireLength += abs(maxX - minX) + abs(maxY - minY);
    }
    return wireLength;
}

void swap(vector<vector<int> >& grid, vector<vector<int> >& savedpos,  int row1, int col1, int row2, int col2) {
    
    int temp1 = grid[row1][col1];
    int temp2 = grid[row2][col2];
    grid[row1][col1] = temp2;
    grid[row2][col2] = temp1;

    // savedpos[index=temp] = {row,col}
    if (temp1 != -1) {
        savedpos[temp1][0] = row2;
        savedpos[temp1][1] = col2;
    }
    if (temp2 != -1) {
        savedpos[temp2][0] = row1;
        savedpos[temp2][1] = col1;
    }
}

void simulatedAnnealing(vector<vector<int> >& savedpos, vector<vector<int> >& components, vector<vector<int> >& grid, int& netsNum, int movesPerTemp) {
    
    // vector <int> TWL; 
    // vector <double> temperatures; 
    // vector <int> iter; 
    // ofstream tempFile("temps.txt");
    // ofstream twlFile("twl.txt");
    // ofstream itFile("it.txt");

    int currentWirelength = calculateWireLength(savedpos, components);
    cout <<endl<< "Initial Wirelength: " << currentWirelength << endl<<endl;
    
    double initialTemp = 500.0 * currentWirelength;             
    double finalTemp = 5e-6 * (currentWirelength / netsNum);  
   
    double currentTemp = initialTemp;
    int iteration = 0;

    int newWireLength = 0; 
    int deltaL = 0;

    //random_device rd;
    mt19937 gen(2);

    while (currentTemp > finalTemp) {
        for (int move = 0; move < movesPerTemp; move++) {
            int rowsNum = grid.size();          //get number of rows in grid
            int colsNum = grid[0].size();       //get number of cols in grid

            uniform_int_distribution<int> rowDist(0, rowsNum - 1);
            uniform_int_distribution<int> colDist(0, colsNum - 1);

            int row1 = rowDist(gen);                            //randomly select 2 grid locations to swap places
            int col1 = colDist(gen);
            int row2 = rowDist(gen);
            int col2 = colDist(gen);

            swap(grid, savedpos, row1, col1, row2, col2);                       //perform swap 

            newWireLength = calculateWireLength(savedpos, components);      //calculate wirelength of new placement
            deltaL = newWireLength - currentWirelength;                     //calculate difference between wirelengths

            if (deltaL < 0) {
                currentWirelength = newWireLength;
            }            
            else {
                double acceptanceProbability = exp(-deltaL / currentTemp);
                uniform_real_distribution<double> compareValue(0.0, 1.0);  
                
                double randomProbability = compareValue(gen);                   //generate random values between 0 and 1 for random prob

                if (randomProbability < acceptanceProbability) {
                    currentWirelength = newWireLength;                         //accept swap if random prob less than acceptance prob
                } else {
                    swap(grid, savedpos, row1, col1, row2, col2);             //reject swap if random prob greater than acceptance prob
                }
            }
        }
        
        currentTemp *= 0.75;                                       //reduce temp - 0.95 is cooling rate
        // TWL.push_back(currentWirelength);
        // temperatures.push_back(currentTemp);
        // iter.push_back(iteration);
        iteration++;
    }
    
    // for (int i = 0; i < TWL.size(); i++) {                          //store wirelengths in file
    //     twlFile << TWL[i] << endl;
    // }
    // twlFile.close();

    // for (int i = 0; i < temperatures.size(); i++) {                //store temperatures in file
    //     tempFile << temperatures[i] << endl;
    // }
    // tempFile.close();

    // for (int i = 0; i < iter.size(); i++) {                         //store iterations in file
    //     itFile << iter[i] << endl;
    // }
    // itFile.close();

}

void printBinaryFormat(const vector<vector<int> >& grid){
     for (int i = 0; i < grid.size(); i++) {
        for (int j = 0; j < grid[i].size(); j++) {
            
            int component = grid[i][j];
            
            if (component == -1) {
                cout << setw(1) << '1'<<" ";
            } else {
                cout << setw(1) << '0'<<" ";
            }
        }
        cout << endl;
    }
}

int main() {
    int componentsNum, netsNum, rowsNum, colsNum;
    vector<vector<int> > components;
    vector<vector<int> > grid;
    
    readFile("d0.txt", componentsNum, netsNum, rowsNum, colsNum, components);

    // size  numRows = numCells and numCols = 2 (to store for each component & its position)
    vector<vector<int> > savedpos(componentsNum, vector<int>(2, -1)); 

    initialPlacement(rowsNum, colsNum, components, grid, savedpos);

    cout << "Initial Placement:" << endl;
    printPlacement(grid);
    cout<<endl;
    // int initialWireLength = calculateWireLength(savedpos, components);
    // cout <<endl<< "Initial Wirelength: " << initialWireLength << endl<<endl;

    // check on savedpos
    // for (int i = 0; i < savedpos.size(); i++) {
    //     cout << "Component " << i << " position: (" << savedpos[i][0] << ", " << savedpos[i][1] << ")" << endl;
    // }

    cout << "Initial Placement in Binary Format:" << endl;
    printBinaryFormat(grid);

    // double initialTemp = 500.0 * initialWireLength;             
    // double finalTemp = 5e-6 * (initialWireLength / netsNum);      
    int movesPerTemp = componentsNum * 10;                            

    simulatedAnnealing(savedpos, components, grid, netsNum, movesPerTemp);

    cout <<endl<< "Final Placement:" << endl;
    printPlacement(grid);
    cout<<endl; 
    int finalWireLength = calculateWireLength(savedpos, components);
    cout <<endl<< "Final Wirelength: " << finalWireLength << endl<<endl;

    cout << "Final Placement in Binary Format:"<<endl;
    printBinaryFormat(grid);

    return 0;
}