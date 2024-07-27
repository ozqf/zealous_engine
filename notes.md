## renderer

Current render routes

* ZE_StartLoop
	* 'manual' route
		* g_game.Draw
			* ?
		* Platform_SubmitFrame
	* Current route
		* ZScene_Draw - iterate scenes
			* ZScene_WriteDrawCommands
				* ZScene_WriteSceneNoGrouping
					* SetupSceneProjection
					* Write Mesh/Quad/Lines/Box/Text Command
			* ZR_ExecuteCommands
			* Platform_SubmitFrame

