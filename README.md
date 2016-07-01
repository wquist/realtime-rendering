# Realtime Rendering Techniques
Showcases a variety of realtime rendering effects. The main focus of the example is the implementation of Light Propagation Volumes (LPV), as described in the Crytek paper, *Cascaded Light Propagation Volumes for Real Time Indirect Illumination*.

Other techniques implemented include:
* Specular and normal mapping.
* Deferred shading, with a G-Buffer consisting of a albedo/specular map and a normal map.
* Directional lighting with PCF shadow mapping.
* Participating media *(god rays)* and GPU 'dust' particles.
* Screen-space ambient occlusion (SSAO).
* Screen-space reflections, using ray casting.

## Building & Running
The program was built and tested on OSX 10.10, using a 1024 Retina MBP, with Apple LLVM 6 / clang 3.5.
1. Unpack the `zip` archives in `data/textures` and `data/models`.
2. Build the sponza scene by running `make scene sponza` in the project root.
3. Run with `./bin/main`.

The camera can be moved with the `WASD` keys, and rotated with the arrow keys. The directional light can be adjusted using the `IJKL` keys.

Different stages of the pipeline can be viewed with the number keys:
* `0`: View the final composited image.
* `1`: View the albedo components of the G-Buffer.
* `2`: View the specular component of the G-Buffer.
* `3`: View the calculated world pixel positions.
* `4`: View the normal component of the G-Buffer, possibly including normal mapping.
* `5`: View the ambient occlusion contribution.
* `6`: View the reflections contribution.
* `7`: View the shadow map (depth buffer from the POV of the directional light).
* `8`: View a slice of the initial LPV injection.
* `9`: View a slice of the propagated LPV contribution.

Individual effects can be toggled and adjusted with other keyboard shortcuts:
* `B`: Toggle reflections.
* `G`: Toggle LPV global illumination.
* `M`: Toggle shadow mapping.
* `N`: Toggle normal mapping.
* `O`: Toggle ambient occlusion.
* `R`: Toggle god rays / particles.
* `U`: Change the LPV slice viewed with `8`/`9`.

## Screenshots
All screenshots taken on Intel i7 / GTX 750M at 2048x1536.

![](/img/screen01.png?raw=true)

![](/img/screen02.png?raw=true)

![](/img/screen03.png?raw=true)

![](/img/screen04.png?raw=true)

![](/img/screen05.png?raw=true)
