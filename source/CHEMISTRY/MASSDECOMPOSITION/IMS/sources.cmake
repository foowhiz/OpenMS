### the directory name
set(directory source/CHEMISTRY/MASSDECOMPOSITION/IMS)

### list all filenames of the directory here
set(sources_list
Alphabet.C
Element.C
IsotopeDistribution.C
RealMassDecomposer.C
Weights.C
AlphabetTextParser.C
)

### add path to the filenames
set(sources)
foreach(i ${sources_list})
	list(APPEND sources ${directory}/${i})
endforeach(i)

### pass source file list to the upper instance
set(OpenMS_sources ${OpenMS_sources} ${sources})

### source group definition
source_group("Source Files\\CHEMISTRY\\MASSDECOMPOSITION\\IMS" FILES ${sources})
