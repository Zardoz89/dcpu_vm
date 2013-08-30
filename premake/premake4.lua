
solution "dcpu_vm"
    newoption {
             trigger     = "use-dynamic",
             description = "Use dynamics sfml lib (dll/so)"
              }
    configurations { "Release" }
	
    project "dcpu_vm"
       kind "WindowedApp"
       language "C++"
       targetname "dcpu_wm"
       targetextension "exe"
       flags { "Optimize", 
               "OptimizeSize", 
			   "OptimizeSpeed" , 
			   "StaticRuntime" }

       files { "../src/*.hpp", "../src/*.cpp"}

       

       configuration "use-dynamic"
            libdirs { os.findlib("sfml-graphics"), 
                        os.findlib("sfml-window"), 
					    os.findlib("sfml-system") }
					 
            links { "sfml-graphics", "sfml-window" , "sfml-system" }

       configuration "not use-dynamic"
            libdirs  { os.findlib("sfml-graphics-s"), 
                         os.findlib("sfml-window-s"), 
					     os.findlib("sfml-system-s") }

            defines { "SFML_STATIC" }					 
            links { "sfml-graphics-s", "sfml-window-s" , "sfml-system-s" }




