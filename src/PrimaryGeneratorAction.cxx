/*
 * PrimaryGeneratorAction.cxx
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


#include "PrimaryGeneratorAction.hh"

#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"

PrimaryGeneratorAction::PrimaryGeneratorAction() 
/*
 * Calls the G4VUserPrimaryGeneratorAction constructor which will halt
 * the execution of the program if PrimaryGeneratorAction is initialized
 * to the runManager before PhysicsList is.
 * 
 * */
 : G4VUserPrimaryGeneratorAction()

{
	
	/*
	 * 
	 * Default constructor just makes a single particle gun with a
	 * muon as the primary particle.
	 * 
	 * It starts at the origin and moves in the positive z direction
	 * */
	G4int numParticles = 1;
	this->ParticleGun = new G4ParticleGun(numParticles);
	
	G4ParticleTable *ParticleTable = G4ParticleTable::GetParticleTable();
	
	
	this->ParticleGun->SetParticleDefinition(ParticleTable->FindParticle("mu+"));
	this->ParticleGun->SetParticleEnergy(1.*MeV);
	this->ParticleGun->SetParticlePosition(G4ThreeVector(0, 0, 0));
	this->ParticleGun->SetParticleMomentumDirection(G4ThreeVector(0, 0, 1));
	
}

PrimaryGeneratorAction::PrimaryGeneratorAction(G4String ParticleName,
                                               G4double Energy,
                                               G4ThreeVector Position,
                                               G4ThreeVector MomentumDirection) 
 : G4VUserPrimaryGeneratorAction()
{
	
	
   
}

PrimaryGeneratorAction::~PrimaryGeneratorAction() {
	
	delete this->ParticleGun;
	
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event *event) {
	
	this->ParticleGun->GeneratePrimaryVertex(event);
	
}