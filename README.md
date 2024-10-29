# Ray Tracer

This project is a ray tracer built as part of a Computer Graphics course. It implements standard ray tracing capabilities and several advanced features to enhance the realism and flexibility of rendered scenes.

## Final Render
![](final_result/Final.jpg) 

## Features

### Alpha Masking
Alpha masking allows for transparency effects, giving materials a realistic translucent appearance. Ideal for rendering objects like glass, water, or other partially transparent materials.  
#### Before
![Alpha Masking - Before](features/alpha/without_alpha_mask.png)  
#### After
![Alpha Masking - After](features/alpha/with_alpha_mask.png)

### Area Lights
Supports area lights, allowing soft shadows and realistic lighting effects by simulating light sources with physical dimensions rather than point lights.  
#### Before
![Area Lights - Before](features/area_lights/without_area_lights.png)  
#### After
![Area Lights - After](features/area_lights/with_area_lights.png)

### Denoising
Integrated denoising capabilities to reduce noise in rendered images, particularly helpful for scenes with complex lighting and shadows.  
#### Before
![Denoising - Before](features/denoising/without_denoise.png)  
#### After
![Denoising - After](features/denoising/with_denoise.png)

### Shading Normals
Utilizes shading normals for improved surface appearance, enhancing the realism of materials under different lighting conditions.  
#### Before
![Shading Normals - Before](features/shading_normals/without_normalmap.png)  
#### After
![Shading Normals - After](features/shading_normals/with_normalmap.png)

### Thin Lens
Simulates depth of field effects with a thin lens model, providing realistic focus and blurring effects in scenes.  
![Thin Lens - Before](features/thinlens/with_thin_lens.png)  

### Volumes
Supports volumetric rendering for fog, smoke, and other participating media, enhancing scene atmosphere.  
#### Before
![Volumes - Before](features/volumes/without_volume.png)  
#### After
![Volumes - After](features/volumes/with_volume.png)
