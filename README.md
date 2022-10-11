# UTC Shader Library for Unreal Engine.

Easily create Master Materials for Unreal Engine !

For Unreal Engine 5.0.

Feel free to send me feedbacks to improve this plugin ! 

## Main Links

If you like my work, you can support me here: https://www.paypal.com/paypalme/UTCompiler 

Marketplace: https://www.unrealengine.com/marketplace/en-US/product/utc-shader-library

Trailer: https://www.youtube.com/watch?v=v8OMGxTqgyI

Demo : https://www.youtube.com/watch?v=AxLDbQTHUMM

Follow the plugin’s development: 
* Twitter: https://twitter.com/UTCompiler 
* Trello: https://trello.com/b/w91ommLH/utc-shader-library

## Release UTC Shader Library 1.1

### Warning

If you added new material functions or modified default ones, save them and Data Tables before upgrade to v1.1 due to the big difference between v1.0 and v1.1.

### What’s news?

* `Presets:` Create TreeView functions in one click by selecting presets and add your owns.
* `Custom Packed Functions:` if you have packed textures, it gives you the possibility to create custom functions to fit with them.
* `List Organization:` Lists that contain material functions in conbo boxes are grouped for more visibility.
* `UI`
* `Add to:` You can now add directly functions in an existing material.
* `Material function library:` Adding new MF like detailed functions, Fuzzy, PDO …


## What is it used for?

The UTC Shader Library helping you to create Master Materials and saving time by avoiding repetitive operations with a library of common material functions, an automation of nodes and parameters creation, and a material instances elements’ organization like autocomplete material group names and sort priorities.

This Shader Library is for UE and shaders beginners, but also advanced users who want to speed up shaders’ creation.

This plugin is composed of 2 different parts,

* `The Master Materials Generator` based on the Material Functions Library
* `The Material Functions Library`

## Plugin Installation

Download the repo as zip and extract it under your project plugins folder as `YourProject/Plugins`. 

In Unreal, go to: `Edit->Plugins`, find UTC Shader Library, enable it and reload the Engine.

Visual Studio will be required.

# Master Materials Generator

You can find the Master Materials Generator in: `Window->Get Content->UTC Materials Generator`

![](/Doc/Screens/UI/OpenMMG.png)

## Concept

The MMG’s main idea is creating master materials by overlapping consecutive layers of material attributes and masks composed of material functions, to set the desired level of details to your material.

![](/Doc/Screens/Masks/MaskConcept.png)

Of course, you can create a simple material with only one material attribute. 

This tool wants to be customizable too, you can add your own material functions for every node type. 

Materials generated by the MMG are regular materials, so they could be a good base to create other advanced materials.

## UI

### Tree View

The Tree View module is the plugin’s heart. Here you can add material attributes and masks and create connections between them.

![](/Doc/Screens/UI/TreeView1.png)

In the main header, select if you want to create a material attribute or a mask in the Function Type Combo Box and click on the Plus Button to add it to the list.

![](/Doc/Screens/UI/RegularHeader.png)

This new parent header is composed of:
* The Function name, named Function_x by default
* The UVs Combo Box, where you can select the UVs type you want, to affect material function texture parameters
* A text which marking what Function type is
* The Plus Button, to add function elements

**Material attribute parent header:**

You can add the elements’ number you want. Each added combo box defines a material function connected to the Material Attribute. 
Adding two times the same or an empty material function will return an error.
At the combo box’s right side, you can check/uncheck the box, if you previously selected an UVs type, to enable/disable the UVs connection with the function’s parameters.

![](/Doc/Screens/UI/MAHeader.png)

**Mask parent header:**

You can add only one element. This element is composed by one main combo box which allow you to select the mask type and 2 other combo boxes, to select which function to blend with. 

It gives the possibility to blend 2 material attributes or 2 masks or one material attribute with another mask.

![](/Doc/Screens/UI/MaskHeader.png)

**Presets:**

By right clicking under TreeView, you can open the preset menu. It gives you the possibility of creating TreeView elements faster. 

![](/Doc/Screens/UI/Preset1.png)

You can add your own presets by selecting the menu header’s.

A message box will open; enter the preset name’s and his menu category (Select an existing one or write your own). This new preset is now available in the preset menu.

![](/Doc/Screens/UI/Preset2.PNG)

You can manage your preset in the Preset Object located in `Plugin-> UTC Shader Library Content-> Master Materials Generator-> P_MainPresets`.

Warning: You can change every preset parameters’ at your own risk.

![](/Doc/Screens/UI/Preset3.PNG)


### Graph

This graph is on read only statement, he reacts to the TreeView.

![](/Doc/Screens/UI/Graph1.png)

He displays 4 custom nodes
* Material Attribute node
* Mask node 
* UVs node
* Material output node
, shows connections between nodes and layers’ direction. 

### Material Settings

Same as in a classic material. When the material is generated, he will have these selected parameters.

![](/Doc/Screens/UI/MaterialSettings.png)

### Main Settings

**Generate Material**

Here, it’s possible to set the Material Name that the generated material will have, his content browser generation path and if you want to generate a Material Instance with it.
![](/Doc/Screens/UI/GenerateMainSettings.PNG)

**Add to Material**

You can add the current TreeView functions directly in an existing material. Drag and drop the target material, push the button and voila !
![](/Doc/Screens/UI/AddMainSettings.PNG)

## Add Custom Material Functions
### Data Tables

For each tree view combo box, it’s possible to add custom material functions. There are 3 Data Tables that correspond to material attributes, masks and UVs. 

They are located in: `Plugins->UTC Shader Library Content->Master Materials Generator`
* The Row Name will be the display name in combo boxes of MMG. 
* Drag and drop your material function into the slot Material Function.
* The Material Attribute Type defines which material attribute input the material function will be connected to. 
* The Material Group which will be displayed in the material instance arrangement.
* The MMG Menu Category will class the material function in the right category in TreeView combo boxes’.

![](/Doc/Screens/UI/DT.png)

### Custom Material Function Specifications

Some cases to know when you create a custom material function and want to use it with MMG:
* In case of using a Vector or Scalar to plug a Texture Sampler, the material function’s input name needs to start with `Texture` to be detected.
* To set a custom default value/description/texture sample type in material instance parameters, check `Use Preview As Default Value` in the material function input node and connect a node with the desired value in the preview input. 

# Material Function Library

You can find all material functions in the content browser: 

`Plugins->UTC Shader Library Content->Material Library->Material Functions`

## Masks List

### Baked Texture
> MF_BakedMask

![](/Doc/Screens/Masks/Mask_Baked.png)

Adding baked textures like curvature, AO or thickness as a mask.

### Distance Field
>MF_MaskByDistanceField

![](/Doc/Screens/Masks/Mask_DistanceField.png)

Tip: Disable `Affect Distance Field Lighting` in your object details panel to remove self distance field. 

### Edge Detection
>MF_EdgeDetectionByVertexNormal

![](/Doc/Screens/Masks/Mask_Edges.png)

This mask reacts with bevels strength, sharp angles will not be detected.

### Light Direction
>MF_LightMask

![](/Doc/Screens/Masks/Mask_LightDirection.png)

This mask is generated by the Directional Light [0] direction’s.

### Object Position
>MF_MaskByObjectPosition

![](/Doc/Screens/Masks/Mask_ObjectPosition.png)

Set manually offset and rotation, stays proportional by position. 

### Simple Noise
>MF_SimpleNoiseVariation

![](/Doc/Screens/Masks/Mask_SimpleNoise.png)

Adding simple noise variation as mask.

### Vertex Normal
>MF_MaskByVertexNormal

![](/Doc/Screens/Masks/Mask_VertexNormal.png)

Get Z normal direction, to add dust, snow … on the mesh.

## Material Outputs List

>Base Color

>Detail Base Color

>Metallic

>Detail Metallic

>Specular

>Detail Specular

>Roughness

>Detail Roughness

>Emissive

>Opacity

>Opacity Mask

>Normal

>Detail Normal

>AO

>Subsurface

>Fuzzy

>Distance Field PDO

## UVs
>Local

>Local no Tiling

>Triplanar