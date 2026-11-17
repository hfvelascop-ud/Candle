# Creating a height map

Because PCB milling is performed at depths of a few hundredths of a millimetre, it is recommended to first scan the surface irregularities and modify the original G-code accordingly.

Required steps:

- Make sure the Z work zero is on the surface of the workpiece.  
- Click the **"Create"** button on the **"Heightmap"** panel.  
- Click **"Auto"** to set the map boundaries; ensure the G-code stays within them.  
- Define the height-map reference point. You will have to re-zero Z at this exact point before running the G-code. If several G-codes are used for one workpiece, make sure this point remains probe-accessible after intermediate operations.  
- Configure the probing grid: set the number of probe points (e.g. 3 × 4), safe Z height (Zt = 5), probe depth (Zb = –1), and feed rate (F = 50).  
- Configure the interpolation grid; values should be at least 10× the number of probe points. This affects machining accuracy on the workpiece surface.  
- While in **Edit** mode, select **Bicubic** (legacy default) or **Bilinear**. The selected method is used for both the preview and the transformed G-code and is saved with the map. Bilinear uses the four surrounding measured heights without cubic overshoot. It does not improve probe repeatability or correct an incorrect work zero. Keep the complete cutting path inside the measured area; outside it the bilinear method holds the nearest edge height instead of extrapolating.
- Press the **"Probe"** button in the **"Heightmap"** window and wait for the probing to finish.  
- If several G-codes are used for the same workpiece, save the map via **"File ▸ Save"** while the height-map editor is open. For subsequent G-codes load the saved map with the **"Open"** button on the **"Heightmap"** panel.  
- Press **"Edit"** on the **"Heightmap"** panel to exit the editor.  
- Enable the **"Use heightmap"** flag to modify the G-code.  
- Set the Z work zero for the obtained height map: raise the tool to a safe height, press **"Origin"** on the **"Heightmap"** panel to move to the reference XY point, then set Z manually or with the probe command from **"User Commands"**.