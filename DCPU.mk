##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Release
ProjectName            :=DCPU
ConfigurationName      :=Release
WorkspacePath          := "/home/luis/Repos/dcpu/DCPU"
ProjectPath            := "/home/luis/Repos/dcpu/DCPU"
IntermediateDirectory  :=./Release
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=Luis Panadero Guardeño
Date                   :=18/08/13
CodeLitePath           :="/home/luis/.codelite"
LinkerName             :=g++
SharedObjectLinkerName :=g++ -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.o.i
DebugSwitch            :=-gstab
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputFile             :=$(IntermediateDirectory)/$(ProjectName)
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E 
ObjectsFileList        :="DCPU.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            :=  -pthread 
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). 
IncludePCH             := 
RcIncludePath          := 
Libs                   := 
ArLibs                 :=  
LibPath                := $(LibraryPathSwitch). 

##
## Common variables
## AR, CXX, CC, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := ar rcus
CXX      := g++
CC       := gcc
CXXFLAGS :=  -std=c++11 -Wno-packed-bitfield-compat -pthread  -O3 -Wall $(Preprocessors)
CFLAGS   :=  -Wno-packed-bitfield-compat -O2 -Wall $(Preprocessors)


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/main$(ObjectSuffix) $(IntermediateDirectory)/dcpu$(ObjectSuffix) $(IntermediateDirectory)/fake_lem1802$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

$(IntermediateDirectory)/.d:
	@test -d ./Release || $(MakeDirCommand) ./Release

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/main$(ObjectSuffix): main.cpp $(IntermediateDirectory)/main$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/luis/Repos/dcpu/DCPU/main.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/main$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/main$(DependSuffix): main.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/main$(ObjectSuffix) -MF$(IntermediateDirectory)/main$(DependSuffix) -MM "main.cpp"

$(IntermediateDirectory)/main$(PreprocessSuffix): main.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/main$(PreprocessSuffix) "main.cpp"

$(IntermediateDirectory)/dcpu$(ObjectSuffix): dcpu.cpp $(IntermediateDirectory)/dcpu$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/luis/Repos/dcpu/DCPU/dcpu.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/dcpu$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/dcpu$(DependSuffix): dcpu.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/dcpu$(ObjectSuffix) -MF$(IntermediateDirectory)/dcpu$(DependSuffix) -MM "dcpu.cpp"

$(IntermediateDirectory)/dcpu$(PreprocessSuffix): dcpu.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/dcpu$(PreprocessSuffix) "dcpu.cpp"

$(IntermediateDirectory)/fake_lem1802$(ObjectSuffix): fake_lem1802.cpp $(IntermediateDirectory)/fake_lem1802$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/luis/Repos/dcpu/DCPU/fake_lem1802.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/fake_lem1802$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/fake_lem1802$(DependSuffix): fake_lem1802.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/fake_lem1802$(ObjectSuffix) -MF$(IntermediateDirectory)/fake_lem1802$(DependSuffix) -MM "fake_lem1802.cpp"

$(IntermediateDirectory)/fake_lem1802$(PreprocessSuffix): fake_lem1802.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/fake_lem1802$(PreprocessSuffix) "fake_lem1802.cpp"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) $(IntermediateDirectory)/main$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/main$(DependSuffix)
	$(RM) $(IntermediateDirectory)/main$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/dcpu$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/dcpu$(DependSuffix)
	$(RM) $(IntermediateDirectory)/dcpu$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/fake_lem1802$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/fake_lem1802$(DependSuffix)
	$(RM) $(IntermediateDirectory)/fake_lem1802$(PreprocessSuffix)
	$(RM) $(OutputFile)
	$(RM) ".build-release/DCPU"


