/*
 * This File is part of Pindel; a program to locate genomic variation.
 * https://trac.nbic.nl/pindel/
 *
 *   Copyright (C) 2011 Kai Ye
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <vector>
#include "genotyping.h"
#include "bam2depth.h"
#include "assembly.h"
#include "reader.h"
#include "pindel.h"
#include "farend_searcher.h"
#include <map>
#include <string>
#include <utility>

void doGenotyping (ControlState & CurrentState, ParCollection & par) {
    // step 1 load whole genome sequences into memory
    
    std::map<std::string,int> ChrName2Index;
    
    std::cout << "Get whole genome sequence..." << std::endl;
    // step 1. get the whole genome sequence
    
    std::vector <Chromosome> AllChromosomes;
    getWholeGenome(CurrentState, AllChromosomes);
    
    for (unsigned i = 0; i < AllChromosomes.size(); i++) {
        std::cout << "ChrName " << AllChromosomes[i].ChrName << "\tChrSeqSize " << AllChromosomes[i].ChrSeq.size() << std::endl;
        ChrName2Index[AllChromosomes[i].ChrName] = i;
    }
    
    
    // step 2 load all variants into memory
    
    // step 2. get all SVs
    //CurrentState.inf_AssemblyInput.open(par.inf_AssemblyInputFilename.c_str());
    std::cout << "\nGet all SVs to assemble..." << std::endl;
    std::vector <Genotyping> AllSV4Genotyping;
    Genotyping OneSV;
    unsigned SV_Count = 0;
    while (CurrentState.inf_GenotypingInput >> OneSV.Type
           >> OneSV.ChrA
           >> OneSV.PosA
           >> OneSV.CI_A
           >> OneSV.ChrB
           >> OneSV.PosB
           >> OneSV.CI_B) {
        OneSV.Index = SV_Count++;
        if (OneSV.ChrA == OneSV.ChrB) {
            if (OneSV.PosA > OneSV.PosB) {
                unsigned Exchange = OneSV.PosA;
                OneSV.PosA = OneSV.PosB;
                OneSV.PosB = Exchange;
            }
        }
        AllSV4Genotyping.push_back(OneSV);
    }
    
    // step 3 define output
    std::string GT_OutputFileName = CurrentState.OutputFolder + "_GT";
    std::ofstream GT_Output(GT_OutputFileName.c_str());
    
    // step 4 for each variant, do genotyping
    for (unsigned SV_index =0; SV_index < AllSV4Genotyping.size(); SV_index++) {
        // step 4.1 if type == DEL, GenotypeDel
        if (AllSV4Genotyping[SV_index].Type == "DEL") GenotypingOneDEL(AllChromosomes, ChrName2Index, CurrentState, par, AllSV4Genotyping[SV_index], GT_Output);
        
        
        
        // step 4.2 if type == DUP, GenotypeDup
        
        // step 4.3 if type == INV, GenotypeINV
        
        // step 4.4 if type == ITX, GenotypeINV
        
        // step 4.5 if type == CTX, GenotypeINV
    }
}

short GenotypingOneDEL(const std::vector <Chromosome> & AllChromosomes, std::map<std::string,int> &ChrName2Index, ControlState & CurrentState, ParCollection & par, Genotyping & OneSV, std::ofstream & GT_Output) {
    
    // get RD signals
    getRelativeCoverage(CurrentState, OneSV);
    //CountRP();
    
    /*
    //std::cout << "AssembleOneSV 1" << std::endl;
    short Max_NT_Size = 30;
    bool WhetherFirstBP = true;
    std::vector <SPLIT_READ> First, Second;
    unsigned SearchCenter;
    unsigned SearchRange;
    std::cout << "Current SV: " << OneSV.Index << " " << OneSV.Type << " " << OneSV.ChrA << " " << OneSV.PosA << " " << OneSV.CI_A 
    << "\t" << OneSV.ChrB << " " << OneSV.PosB << " " << OneSV.CI_B << std::endl;
    // get first BP
    CurrentState.Reads.clear();
    //std::cout << "AssembleOneSV 2" << std::endl;
    
    CurrentState.CurrentChrName = OneSV.ChrA;
    CurrentState.CurrentChrSeq = AllChromosomes[ChrName2Index.find(OneSV.ChrA)->second].ChrSeq;
    CONS_Chr_Size = CurrentState.CurrentChrSeq.size() - 2 * g_SpacerBeforeAfter; // #################
    //std::cout << "CONS_Chr_Size " << CONS_Chr_Size << std::endl;
    g_maxPos = 0; // #################
    g_NumReadInWindow = 0; // #################
    g_InWinPlus = 0; // #################
    g_InWinMinus = 0; // #################
    g_CloseMappedPlus = 0; // #################
    g_CloseMappedMinus = 0; // #################
    unsigned Left, Right;
    if (OneSV.PosA > OneSV.CI_A + 1000)  
        CurrentState.lowerBinBorder = OneSV.PosA - OneSV.CI_A - 1000; //CurrentState.
    else CurrentState.lowerBinBorder = 1;
    CurrentState.upperBinBorder = OneSV.PosA + OneSV.CI_A + 1000;
    Left = OneSV.PosA + g_SpacerBeforeAfter - OneSV.CI_A;
    Right = OneSV.PosA + g_SpacerBeforeAfter + OneSV.CI_A;
    
    std::cout << "\nFirst BP\tChrName " << CurrentState.CurrentChrName << "\tRange " << CurrentState.lowerBinBorder << " " << CurrentState.upperBinBorder << std::endl;
    getReads(CurrentState, par);
    
    //std::cout << "First size: " << CurrentState.Reads.size() << std::endl;
    CombineAndSort(AllChromosomes, ChrName2Index, CurrentState, par, OneSV, First, CurrentState.lowerBinBorder, CurrentState.upperBinBorder, WhetherFirstBP);
    
    CleanUpCloseEnd(First, Left, Right); // vector of reads
    
    std::cout << "\nFirst size " << First.size() << std::endl;
    SearchRange = OneSV.CI_B + 1000;
    SearchCenter = OneSV.PosB + g_SpacerBeforeAfter;
    Left = OneSV.PosB + g_SpacerBeforeAfter - OneSV.CI_B;
    Right = OneSV.PosB + g_SpacerBeforeAfter + OneSV.CI_B;
    
    for (unsigned ReadIndex = 0; ReadIndex < First.size(); ReadIndex++) {
        First[ReadIndex].FarFragName = OneSV.ChrB;
        SearchFarEndAtPos(AllChromosomes[ChrName2Index.find(OneSV.ChrB)->second].ChrSeq, First[ReadIndex], SearchCenter, SearchRange);
    }
    //std::cout << "AssembleOneSV 7" << std::endl;
    
    CleanUpFarEnd(First, Left, Right);
    //std::cout << "AssembleOneSV 8" << std::endl;
    
    
    for (unsigned ReadIndex = 0; ReadIndex < First.size(); ReadIndex++) {
        if (First[ReadIndex].UP_Close.size()) {
            if (First[ReadIndex].UP_Far.size()) {
                //if (First[ReadIndex].UP_Far[0].LengthStr < 0) continue;
                //std::cout << "First UP_Far: ";
                //std::cout << First[ReadIndex].UP_Far.size() << std::endl;
                //std::cout << "First[ReadIndex].UP_Far.size() " << First[ReadIndex].UP_Close[First[ReadIndex].UP_Close.size() - 1].LengthStr << std::endl;
                if (First[ReadIndex].UP_Far[First[ReadIndex].UP_Far.size() - 1].LengthStr + First[ReadIndex].UP_Close[First[ReadIndex].UP_Close.size() - 1].LengthStr + Max_NT_Size >= First[ReadIndex].ReadLength) OutputCurrentRead(AllChromosomes, ChrName2Index, CurrentState, par, OneSV, First[ReadIndex], ASM_Output);
            }
            else if (First[ReadIndex].UP_Far_backup.size()) {
                //std::cout << "First UP_Far_backup ";
                //std::cout << First[ReadIndex].UP_Far_backup.size() << std::endl; //First[ReadIndex].UP_Far_backup[First[ReadIndex].UP_Far_backup.size() - 1].LengthStr << " " << First[ReadIndex].UP_Close[First[ReadIndex].UP_Close.size() - 1].LengthStr << " " << Max_NT_Size << " " << First[ReadIndex].ReadLength << std::endl;
                //if (First[ReadIndex].UP_Far_backup[First[ReadIndex].UP_Far_backup.size() - 1].LengthStr + First[ReadIndex].UP_Close[First[ReadIndex].UP_Close.size() - 1].LengthStr + Max_NT_Size >= First[ReadIndex].ReadLength) 
                //std::cout << "First[ReadIndex].UP_Far_backup.size() # " << First[ReadIndex].UP_Close[First[ReadIndex].UP_Close.size() - 1].LengthStr << std::endl;
                OutputCurrentRead(AllChromosomes, ChrName2Index, CurrentState, par, OneSV, First[ReadIndex], ASM_Output);
            }
        }
    }
    
    
    // get second BP
    CurrentState.Reads.clear();
    WhetherFirstBP = false;
    CurrentState.CurrentChrName = OneSV.ChrB;
    CurrentState.CurrentChrSeq = AllChromosomes[ChrName2Index.find(OneSV.ChrB)->second].ChrSeq;
    CONS_Chr_Size = CurrentState.CurrentChrSeq.size() - 2 * g_SpacerBeforeAfter; // #################
    g_maxPos = 0; // #################
    g_NumReadInWindow = 0; // #################
    g_InWinPlus = 0; // #################
    g_InWinMinus = 0; // #################
    g_CloseMappedPlus = 0; // #################
    g_CloseMappedMinus = 0; // #################
    if (OneSV.PosB > OneSV.CI_B + 1000)  
        CurrentState.lowerBinBorder = OneSV.PosB - OneSV.CI_B - 1000;
    else CurrentState.lowerBinBorder = 1;
    CurrentState.upperBinBorder = OneSV.PosB + OneSV.CI_B + 1000;
    Left = OneSV.PosB + g_SpacerBeforeAfter - OneSV.CI_B;
    Right = OneSV.PosB + g_SpacerBeforeAfter + OneSV.CI_B;
    
    std::cout << "\nSecond BP\tChrName " << CurrentState.CurrentChrName << "\tRange " << CurrentState.lowerBinBorder << " " << CurrentState.upperBinBorder << std::endl;    
    getReads(CurrentState, par);
    
    CombineAndSort(AllChromosomes, ChrName2Index, CurrentState, par, OneSV, Second, CurrentState.lowerBinBorder, CurrentState.upperBinBorder, WhetherFirstBP);
    
    CleanUpCloseEnd(Second, Left, Right);
    
    std::cout << "\nSecond size " << Second.size() << std::endl;
    SearchRange = OneSV.CI_A + 1000;
    SearchCenter = OneSV.PosA + g_SpacerBeforeAfter;
    Left = OneSV.PosA + g_SpacerBeforeAfter - OneSV.CI_A;
    Right = OneSV.PosA + g_SpacerBeforeAfter + OneSV.CI_A;
    
    for (unsigned ReadIndex = 0; ReadIndex < Second.size(); ReadIndex++) {
        Second[ReadIndex].FarFragName = OneSV.ChrA;
        SearchFarEndAtPos(AllChromosomes[ChrName2Index.find(OneSV.ChrA)->second].ChrSeq, Second[ReadIndex], SearchCenter, SearchRange);
    }
    
    CleanUpFarEnd(Second, Left, Right);
    
    for (unsigned ReadIndex = 0; ReadIndex < Second.size(); ReadIndex++) {
        if (Second[ReadIndex].UP_Close.size()) {
            if (Second[ReadIndex].UP_Far.size()) {
                //std::cout << "Second UP_Far: " << Second[ReadIndex].UP_Far.size() << std::endl;
                //std::cout << "Second[ReadIndex].UP_Far.size() " << Second[ReadIndex].UP_Close[Second[ReadIndex].UP_Close.size() - 1].LengthStr << std::endl;
                if (Second[ReadIndex].UP_Far[Second[ReadIndex].UP_Far.size() - 1].LengthStr + Second[ReadIndex].UP_Close[Second[ReadIndex].UP_Close.size() - 1].LengthStr + Max_NT_Size >= Second[ReadIndex].ReadLength) OutputCurrentRead(AllChromosomes, ChrName2Index, CurrentState, par, OneSV, Second[ReadIndex], ASM_Output);
            }
            else if (Second[ReadIndex].UP_Far_backup.size()) {
                //std::cout << "Second UP_Far_backup " << Second[ReadIndex].UP_Far_backup.size() << std::endl; //<< Second[ReadIndex].UP_Far_backup[Second[ReadIndex].UP_Far_backup.size() - 1].LengthStr << " " << Second[ReadIndex].UP_Close[Second[ReadIndex].UP_Close.size() - 1].LengthStr << " " << Max_NT_Size << " " << Second[ReadIndex].ReadLength << std::endl;
                //if (Second[ReadIndex].UP_Far_backup[Second[ReadIndex].UP_Far_backup.size() - 1].LengthStr + Second[ReadIndex].UP_Close[Second[ReadIndex].UP_Close.size() - 1].LengthStr + Max_NT_Size >= Second[ReadIndex].ReadLength) 
                //std::cout << "Second[ReadIndex].UP_Far_backup.size() " << Second[ReadIndex].UP_Close[Second[ReadIndex].UP_Close.size() - 1].LengthStr << std::endl;
                OutputCurrentRead(AllChromosomes, ChrName2Index, CurrentState, par, OneSV, Second[ReadIndex], ASM_Output);
            }
        }
    }
    //std::cout << "AssembleOneSV 15" << std::endl;
    
    unsigned SumSize = 0;
    for (unsigned ReadIndex = 0; ReadIndex < First.size(); ReadIndex++) {
        SumSize += First[ReadIndex].UP_Far.size() + First[ReadIndex].UP_Far_backup.size();
    }
    for (unsigned ReadIndex = 0; ReadIndex < Second.size(); ReadIndex++) {
        SumSize += Second[ReadIndex].UP_Far.size() + Second[ReadIndex].UP_Far_backup.size();
    }
    if (SumSize == 0 && OneSV.ChrA == OneSV.ChrB) {
        TryLI(AllChromosomes, ChrName2Index, CurrentState, par, OneSV, First, Second, ASM_Output);
    }
    */
    return 0;
}


