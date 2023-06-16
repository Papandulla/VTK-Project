# VTK-Project
This is the VTK project for the course Data Visualization. 

Part 1B
Question 1 Iso Values
    vtkHeadDataset.cxx
        Without Color Transfer Function:
        2 important surfaces.
        1 surface is for the skin (in skin color).
        The opacity is lower such that the skin is half opaque to visualize the structure of the head better.
        1 surface is for the skull details (in gray color).
        No opacity such that the bone is opaque in gray color.

    Teapot.cxx
        2 important surfaces.
        1 represents the external surface of the teapot.
        The opacity is set to be half opaque to visualize the structure of the teapot better.
        1 represents the internal surface of the teapot.
        The opacity is set to be half opaque.

    Foot.cxx
        2 important surfaces.
        1 surface is for the skin (in skin color).
        The opacity is set to be half opaque to visualize the structure of the foot better.
        1 surface is for the bone details (in gray color).
        The opacity is set to be half opaque.

    Other Volume Data: MRbrain.cxx
        1 important surface.
        The surface is for the brain (in pink color).
        The opacity is set to be half opaque to visualize the structure of the brain better.

Question 3 User Manual
    The program starts with the mode “IsoSurface” in the beginning. The user can toggle the rendering mode into “rayMarching” by pressing ‘t’ key and convert back to “IsoSurface” mode by pressing SHIFT + ‘t’ together. 

    Controller:
        Key ‘e’/’q’ = Exit the program.
        Keypress ‘s’ = Switch to surface visual representation
        Keypress ‘w’ – Switch to wireframe visual representation.
        Key ‘t’ = Switch into rayMarching rendering mode
        Key ‘T’ = Switch into IsoSurface rendering mode
        Key ']' = Increment the iso values by 100.
        Key '[' = Decrement the iso values by 100.
        Key '>' = Increment the ray step size by 10.
        Key '<' = Decrement the ray step size by 10.
        Key '+' = Increment the RGBpoint of the fifth color in the color transfer function by 10
        Key '-' = Decrement the RGBpoint of the fifth color in the color transfer function by 10.
        Key 'p' = Increment the opacity of the fifth value in the opacity transfer function by 10
        Key 'P' = Decrement the opacity of the fifth value in the opacity transfer function by 10.
        Scroll up = Zoom in
        Scroll down = Zoom out
        Left click = Rotate the visualized volume data according to the position of the cursor.

