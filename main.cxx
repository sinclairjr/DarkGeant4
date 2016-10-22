/*
 * main.cxx
 * 
 * Copyright 2016 Emma Davenport <Davenport.physics@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

//Geant4 Headers
#include "G4RunManager.hh"
#include "G4ThreeVector.hh"
#include "G4VUserDetectorConstruction.hh"
#include "QGSP_BERT.hh"

#ifdef G4VIS_USE

	#include "G4VisExecutive.hh"
	void InitializeVisManager();
	static G4VisManager *vis;
	
#endif
#ifdef G4UI_USE

	#include "G4UImanager.hh"
	void InitializeUIManager();
	static G4UImanager  *ui;
	
#endif

//User Headers
#include "DetectorConstruction.hh"
#include "PhysicsList.hh"
#include "PrimaryGeneratorAction.hh"
#include "Utilities.hh"
#include "LuaPlugin.hh"
#include "SteppingAction.hh"

// C & C++ Headers
#include <cstring>
#include <string>
#include <vector>

/* 
 * ~~~~~~~~~~
 * namespaces
 * ~~~~~~~~~~
 * 
 * */
using std::string;
using std::vector;
using std::cout;
using std::cin;

/*
 * ~~~~~~~~~~~~~~~~
 * Static variables
 * ~~~~~~~~~~~~~~~~
 * 
 * */
 
static bool TerminalOutput = true;

static string Module = "config";
static vector<G4String> ExecutionVector;
static FourVectorStruct<G4double> *JBStruct = NULL;

static ConfigLuaInstance *ConfigFileInstance = NULL;
static DetectorConfigLuaInstance *DetectorConfigFileInstance = NULL;

static DetectorConstruction *Detector = NULL;

static G4RunManager *runManager;

/*
 *  ~~~~~~~~~~~~~~~~~~~
 *  Function Prototypes
 *  ~~~~~~~~~~~~~~~~~~~
 * 
 * */

void HandleArguments(int argc, char *argv[]);
void InitializeState();
void InitializeLuaInstances();
void InitializeRunManager(G4RunManager *runManager);

// Relies on DetectorConfigLua instance
void InitializeDetectorGeometry();

void Clean();

int main(int argc, char *argv[]) {
	
	HandleArguments(argc, argv);
	InitializeState();
	
	std::cin.get();
	
	Clean();
	
	return 0;
}

/*
 * InitializeState
 * 
 * * Description
 * 
 * 		Initializes important runtime variables, calls several important
 * 		functions that initialize the UI and VIS functions 
 * 		(if compiled in), and changes runtime state depending on user 
 * 		input.
 * 
 * 
 * */

void InitializeState() {
	
	runManager = new G4RunManager();
	
	InitializeLuaInstances();
	InitializeRunManager(runManager);
#ifdef G4VIS_USE
	InitializeVisManager();
#endif
#ifdef G4UI_USE
	InitializeUIManager();
	if (ExecutionVector.size() != 0)
		cin.get();
	for (size_t i = 0; i < ExecutionVector.size();i++)
		ui->ApplyCommand(ExecutionVector[i]);		
#endif
	
	
}

/*
 * Clean()
 * 
 * * Description
 * 
 * 		...
 * 
 * */

void Clean() {
	
#ifdef G4UI_USE
	delete ui;
#endif
#ifdef G4VIS_USE
	delete vis;
#endif	

	/*
	 * 
	 * There might be a memory leak upon exiting, but it's nothing
	 * to worry about too much. Any modern operating system should
	 * just reclaim that memory.
	 * 
	 * TODO
	 * 
	 * 		FIX memory leak and possibly the segmentation fault.
	 * 
	 * */
	//delete runManager;

	if (JBStruct != NULL)
		delete JBStruct->array;
	delete JBStruct;
	
	delete ConfigFileInstance;
	delete DetectorConfigFileInstance;
}

/*
 * InitializeLuaInstances()
 * 
 * * Description
 * 
 * 		Uses openmp to introduce parallelization for the lua state
 * 		config files.
 * 
 * * Comment
 * 
 * 		Be sure to define the build type as release!
 * 
 * 		cmake -DCMAKE_BUILD_TYPE=Release -DGeant4_DIR=/path/to/G4 ../
 * 
 * */

void InitializeLuaInstances() {

	/*
	 * 
	 * uses openmp to introduce parallelization for the lua state
	 * config files
	 * 
	 * */
	#pragma omp parallel sections
	{
		#pragma omp section
		{
	
			ConfigFileInstance = new ConfigLuaInstance(Module);
			ConfigFileInstance->CloseLuaState();
		
		}
		#pragma omp section
		{
		
			DetectorConfigFileInstance = new DetectorConfigLuaInstance(Module);
			DetectorConfigFileInstance->CloseLuaState();
		
		}
	}	
	
}

/*
 * InitializeRunManager(G4RunManager *runManager)
 * 
 * * Description
 * 
 * 		...
 * 
 * * Comment
 * 
 * 		You might notice something a bit odd with 

 * 		PrimaryGeneratorAction and SteppingAction. I wasn't able to
 * 		find a good solution for delimiting events in the
 * 		DarkGeantOutput.dat file without having 
 * 		PrimaryGeneratorAction::GeneratePrimaries increment a variable
 * 		in the SteppingAction instance.
 * 
 * 		This seems a bit hackish and I certainly don't like how
 * 		interconnected this code is, so a better solution would be
 *		ideal.
 * 
 * */

void InitializeRunManager(G4RunManager *runManager) {

	runManager->SetUserInitialization(new DetectorConstruction());
	runManager->SetUserInitialization(ConfigFileInstance->physicslist);
	
	PrimaryGeneratorAction *Generator = new PrimaryGeneratorAction();
	runManager->SetUserAction(Generator);
	runManager->SetUserAction(Generator->GetSteppingAction());
	
	runManager->Initialize();
	
}
#ifdef G4UI_USE
void InitializeUIManager() {

	ui = G4UImanager::GetUIpointer();
	if (TerminalOutput) {
		
		ui->ApplyCommand("/run/verbose 1");
		ui->ApplyCommand("/event/verbose 1");
		ui->ApplyCommand("/tracking/verbose 1");
		
	}
	ui->ApplyCommand("/control/execute vis.mac");
	
}
#endif

#ifdef G4VIS_USE
void InitializeVisManager() {

	vis = new G4VisExecutive();
	vis->Initialize();
	
}
#endif

void InitializeDetectorGeometry() {

	
	
}



struct ArgumentTable {
	
	string name;
	void (*Function)(int argc, char *argv[], int index);
	
};

//Argument Function Prototypes
void Execute_Argument(int argc, char *argv[], int index);
void JBInput_Argument(int argc, char *argv[], int index);
void Module_Argument (int argc, char *argv[], int index);
void Limit_T_Argument(int argc, char *argv[], int index);



static const int numHandledArguments = 4;
static const ArgumentTable Table[numHandledArguments] =
{{"-execute", &Execute_Argument},
 {"-JBInput", &JBInput_Argument},
 {"-module" , &Module_Argument},
 {"-lim-t-output", &Limit_T_Argument}};



/*
 * HandleArguments(int argc, char *argv[])
 * 
 * * Description
 * 
 * 		Checks each provided runtime argument provided by the user
 * 		against known runtime arguments and acts accordingly.
 * 
 * */
 
void HandleArguments(int argc, char *argv[]) {
	
	for (int y = 1;y < argc;y++)
		for (int x = 0;x < numHandledArguments;x++)
			if (strcasecmp(argv[y], Table[x].name.c_str()) == 0)
				Table[x].Function(argc, argv, y);		
		
}

/*
 * Execute_Argument(int argc, char *argv[], int index)
 * 
 * * Description
 * 
 * 		...
 * 
 * */
 
void Execute_Argument(int argc, char *argv[], int index) {

	if ((index + 1) == argc) {
	
		cout << "Did not give a good execution argument\n";
		return;
		
	}
	
	G4String CharLiteralToString(argv[index + 1]);
	ExecutionVector.push_back("/control/execute " + CharLiteralToString);
	
}

/*
 * JBInput_Arguments(int argc, char *argv[], int index)
 * 
 * * Description
 * 
 * 		...
 * 
 * */

void JBInput_Argument(int argc, char *argv[], int index) {

	string filename("output.dat");
	JBStruct = Get_VectorStruct_FromFile<G4double>(filename);
	
}

/*
 * Module_Argument(int argc, char *argv[], int index)
 * 
 * * Description
 * 
 * 		...
 * 
 * */


void Module_Argument (int argc, char *argv[], int index) {

	if ((index+1) == argc) {
	
		cout << "Need to specify a module!\n";
		return;
		
	}
	Module = "config/module/" + string(argv[index+1]);
	
}

void Limit_T_Argument(int argc, char *argv[], int index) {

	TerminalOutput = false;
	
}

