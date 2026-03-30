# 4DS ENGINE Change Log
 
## Version 3.0.16
 
### Fixes
- Fix for GUI glitches after resizing app window 
- Set setup rotation default to 0
- Fix for orientation bug in sphere shape, count objects clockwise
- Speaker label visualisation fixes
- ( Fix for routing and position text still being displayed after hiding speakers )
- ( Fix for routing visualisation )
- ( Fix bug: granulator not sounding )
- ( Fix for crash on startup when invalid audio device was selected )

### Features/improvements
- GUI update, icons
- Phantom speakers
- Default routing in xml
- Entity aliases
- Performance gain due to parallel processing of master effects
- Panning smoothing default value set to 3ms
- Hollow parameter in box shape
- Spacing in sphere shape
- Increase articulation exponent
- ( Granulator slope parameter )
- ( Optional speaker labels )
- ( Audio device hotloading )
- Example setup with phantom speaker, default routing and label:

<?xml version="1.0" encoding="UTF-8"?>
<setup version="3.0" routing="2 3 1">

      <speaker id="s1" x="-2" y="1" z="-2" speakerType="satellite" />
      <speaker id="s2" label="optional display label" x="2" y="1" z="-2" speakerType="satellite"  />
      <speaker id="s3" x="-2" y="1" z="2" speakerType="satellite" />
      
      <phantomspeaker id="s4" x="2" y="1" z="2" routing="s2:0.5 s3:0.5" />


      <grid name="quad">
          <shape type="square" speakers="s1 s2 s3 s4" />
          <shape type="projectionLine" speakers="s2 s1" />
          <shape type="projectionLine" speakers="s1 s3" />
          <shape type="projectionLine" speakers="s3 s4" />
          <shape type="projectionLine" speakers="s4 s2" />
      </grid>

</setup>


## Version 3.0.15

- Stijn?

## Version 3.0.14

### Fixes
- Fix for panning glitch for objects with zero and non-zero dimensions

## Version 3.0.13

- Thread-safety improvements? Stijn?

## Version 3.0.12

### Fixes
- Check safety queued OSC events
- Error in line panning shape

### Feature/improvements
- Stereo panner shape with phantom quad panning
- Support higher sample rates when available
## Version 3.0.11

### Fixes
- Fix for edge cases in 1D panning
- Fix for occasional situation where the control panel would get stuck.
- OSC output of multispeaker data works again.

### Features/improvements
- Improved stereo setup.
- Spacing max value increased to 100.
## Version 3.0.10

### Fixes
- Fix for occasional crashes when adding/removing (large numbers of) sound objects.
- Still start up Engine when a setup file is invalid.
- Fix for occasional crash when receiving corrupt OSC
- Potential performance boost on Windows (realtime priority)

### Features/improvements
- Side panel objects sorted alphabetically.
- Rendering enable/disable button.
- Clearer errors for speaker setup mistakes.
- Speaker setup rotation.
- Side panel UI tweaks.
- Row & circle shape with variable object count.

- Breaking change
- The panning shapes XLine, XStartPoint, XEndPoint, YLine, YStartPoint, YEndPoint, ZLine, ZStartPoint, ZEndPoint are removed. Speaker setup files with these shapes should be replaced with the generic Line shape. For example:

- Before:

      <grid name="subs" speakerType="sub">
          <shape type="xStartPoint" speakers="sub1" />
          <shape type="xLine" speakers="sub1 sub2" />
          <shape type="xEndPoint" speakers="sub2" />
      </grid>


- After:

      <grid name="subs" speakerType="sub">
          <shape type=“line” speakers="sub1 sub2" />
      </grid>

- For help, please contact beta@4dsound.net!

## Version 3.0.9

### Fixes
- Sound object names display option works again
- Path visualisation of custom entity types works again
- Spatial delay & feedback delay buffers of inactive particles are automatically being cleared
- Closing the detached control panel window closes the control panel instead of the app
- Fixed scrolling issue in detached control panel
- Ensures the control panel can’t be trapped again behind the header, footer or side bar

### Features/improvements
- Spatial delay & feedback delay noise modulation tweaks
- Various audio bridge & low-level audio performance improvements
- Adjustable panning smoothing setting

- Breaking changes
- Spatial delay & feedback delay noise modulation range is changed to only increase and never decrease the delay times.
- Spatial delay & feedback delay noiseDepth value range is now in ms, from 0-1000.

## Version 3.0.6-test

### Fixes
- Microphone usage requested
- Grid names are automatically converted to lower case to prevent OSC mismatches

### Features/improvements
- Feedback delay effect: dry/wet parameter and modulation tweaks
- Audio bridge improvements

## Version 3.0.4
### Features/improvements
- ‘spacing’ parameter for Box & Custom shape type
- Side panel & setup window redesign
- Add routing visualisation option
- Grid visualisation improvements (floor plane, smaller font, increased display size)
- Updated naming of default speaker setup files
- New feedback delay effect
- Audio bridge receiver improvements: lower real latency, syncing between streams
- Control audio bridge latency manually or calibrate automatically through UI
### Fixes
- Parameters of grids of which the name occurs in multiple setups always work again
- Don’t crash on corrupt OSC input
- Fix input VU meters


- Breaking changes:
- For some setup files, grid names have been changed (satellites>omnigrid, lowercase). These need to be changed in existing live sets.
## Version 3.0.1
### Features/improvements
- Licensing tiers
- Show display names and unique names in parameters and exposed data tabs
- Channel counts up to 256 supported for external inputs (audio bridge and device input)

### Fixes
- Crash when using groups transformations
- Crash when changing samplerate or buffersize



## Version 2.0.14.2 (legacy)


- 2.0.14.1 + fix for master level OSC input.


## Version 2.0.14


### Features/improvements
- Input mode defaults to device
- Box shape in soundentity

### Fixes
- Fix more crashes adding/removing entities
- Fix resonance parameter range for low and highpass filters (0 to 1)
- Fix granulator transposition range
## Version 2.0.13


### Features/improvements
- Dynamically add sound entity types through OSC
- Speaker test with proper pink noise and test tone
- LPF & HPF effects for sound entities, filter with resonance
- Copy-pasting of text in input fields (for routing and license keys)
- Append speaker names to recording file names
- ‘Audio Workgroup’ performance boost on macOS
- Display of scaled speaker positions
- Minimum startup time for splash screen
- Smooth transformation

### Fixes
- Fixed speaker panning visualisation bug
- Crash on speaker setup selection
- Removing panners on entity removal, fixing cause for some crashes.

- Breaking changes:
- The grid “perspectiveFactor” parameter is inverted (0 = orthogonal, 1 = perspective)


## Version 2.0.12


### Features/improvements
- Use a more readable font for the change log ;)
- /environment/clear OSC input message
- Optimization of audiobridge packetsize to lower latency corresponding to sender buffer size.

- Breaking Changes
- Maximum particle count part of SoundEntityTypes in app_structure.json
- Defaulting with empty environment
- inputMode parameter defaults to audiobridge

### Fixes
- Master volume OSC input
- Speaker amplitude OSC output
- Fixed OSC logging known/unknown
- Fixed group naming error
- Fixed send/receive parallelization error resulting in glitches in send/receives and space
- Fixed SpeakerSetupShape including duplicate speaker particles
- Fixed several bugs in removing entities from environment, that resulted in occasional crashes


## Version 2.0.11
- Improvements:
- Speaker test default volume all the way down
## Version 2.0.rc10
### Features/Improvements
- Various thread-safety improvements.
- Update to NAP 0.7.5
- New ‘box’ shape type.
- 4dpan 3.0: ‘perspectiveFactor’ grid setting that gradually alters the perspective projection
- 4dpan 3.0: at-runtime scaling of speaker setups
- 4dpan 3.0: new 'line’ panning shape that works with any amount of speakers and in any 3D direction
- 4dpan 3.0: ‘projectionLine’ & ‘projectionTriangle’ panning shapes works with any order
- 4dpan 3.0: ‘square' & ‘projectionSquare’ panning shapes works in any direction
- 4dpan 3.0: minor improvement (by ~20%) of 'worst case' performance of the panning algorithm.
- Licensing UI: added a License tab, displaying the name and final characters of the serial key.
- Licensing UI: display a 'reverify' button when the Engine needs to be reverified after it has been disconnected to the internet for too long.
- Licensing UI: keep displaying license name after the license has expired or needs reverification.
- Dynamic environment: functionality to spawn and remove predefined sound entity types over OSC
- Make side bar scrollable.
- Adds OSC control for the MultiSpeaker setup master level (OSC address: multispeaker/level).

### Fixes
- Custom shape scaling fixed.
- Fix for stuck panning visualisation when 'master' gets selected
- 4dpan 3.0: fixed issues with 'deformedProjectionPlane' and 'deformedCube' shapes

- Breaking changes (4dpan 3.0):
- Groups now work with speaker names instead of speaker channels. Existing setup files need to be updated.
- Speaker channels now start counting from 1 instead of 0. Existing setup files need to be updated.
- The panning shape ‘SphereShape’ is not available anymore. For now, you can use ‘TetrahedronShapes’ and a ‘phantom speaker’ routing solution instead.
- The 'boost' grid setting is removed
- For detailed instructions, see: 4dpan 3.0 speaker setup file instructions.

- OSC interface (dynamic environment): <br>
/environment/add [type] [name] <br>
/environment/remove [name] <br>
/environment/addGroup [name] <br>
/environment/removeGroup [name] <br>


## Version 2.0.rc9
### Features/Improvements
- Update to NAP 0.7.

## Version 2.0.rc8

### Features/Improvements
- Added a ‘custom’ shape of which individual particles can be controlled via OSC
- Various UI improvements (icons, speaker test UI, clear exposed data selection, sort speaker setup, limiter visualisation)
- Show groups in monitor side panel

### Fixes
- Fix for jumping rotation bug
- Fix for extended temporary licenses
- Fixed erroneous offset of selected entity indicator in monitor side panel
- Fixed data not showing in inspection panel
- Automatically wrap input VU meters horizontally
- Automatically scroll down to the last OSC log entries
- Aligned meters with Ableton's meters

## Version 2.0.rc6

### Features/Improvements
- Display error message when audio device settings fail to start the audio stream

### Fixes
- Rare crash on shutdown fixed
- Napkin no longer fails to open 4DSOUND project file due to TurboActivate DLL search paths on windows.
- Modulating granulator density and duration no longer interrupts the grain timings.
- Some audio host API / device combinations resulted in the wrong device choices on windows.
- Audio device settings weren’t always saved to the configuration file properly.

## Version 2.0.rc5

### Features/Improvements
- Headphones/binaural recording
- Improved recording and playback interface including playback level
- Control headphones listener position with first person monitor camera position
- Apple Silicon compatibility using Rosetta
- MacOS app bundle
### Fixes
- Distance diffusion name fix
- Invert transformation now works with follow transformation

- Other changes:
- Upgrade to NAP 0.5
- Increased font size

- Known issues:
- GUI is scaled incorrectly after dragging the window to a newly connected external monitor. Restarting the Engine fixes the issue.

## Version 2.0.rc4
- Lower latencies on macOS
- Increased stability and performance

## Version 2.0.rc3
### Features/Improvements
- Show selection of sound objects in side panel
- Side panel text indentation fix
- Added a free camera mode
- Clearer error message if license is already in use

### Fixes
- Fix for CPU spikes when muting sound objects
- Only update slider values after typing finished
- Particle labels stay visible when disabling particle visibility
- Prevent crash when setup file was not found.

- Other changes:
- Binaural mode of sound objects disabled by default




## Version 2.0.rc2
### Features/Improvements
- Show selection of sound objects in side panel
- Added a free camera mode
- Clearer error message if license is already in use

### Fixes
- Set orbit camera look-at position to 0 2 0
- Frame rate & control rate texts locked to static position
- Fixed footer text vertical alignment
- Don’t display names of disabled sound objects
- Fixed sound entities inspection bug
- Fixed side panel spacing issue
- Fixed side panel checkboxes display bug


## Version 2.0.rc1
### Features/Improvements
- Vantage point visualisation
- Spatial delay visualisation
- Simplified source sound object type
- “peak decay” algorithm for visualisation level meters
- “cloud box” particle level visualisation
- Improved particle box frames
- Improved “halo” output visualisation
- Spatial delay noise modulation is now absolute instead of relative to other delay scales
- Reverb particles are automatically hidden to not interfere with speaker visualisation
- Point particles are displayed as points
- Improved selection visualisation
- Basic quad, row, cube and circle shapes
- Logging cleanup
- Various side panel design improvements
- Monitor options dropdown menu
- Recording indicator in header
- Simplified audio device GUI
- Rename space to ‘sound entity’, and spatial verb to ‘space’.
- New splash screen

### Fixes
- GUI: slightly lighter shade of eigengrau for better visibility, brighter font color
- Path visualisation offset fix
- Fix for crashes when selecting spirals and python shapes
- Improvement of audio device settings stability
- Don’t show path & spatial delay visualisation of disabled sound objects
- Fix for control rate erroneously indicating 0.
- Allow inspecting reverb sound object
- Particle distribution bug fix (‘occasional channels within a source do not work’)
- Exponential attack/decay for expodec curves in granulator (preventing ticks)
- Also show labels when shape visualisation is disabled
- Speaker channel labels start counting from 1
- Frame rate performance improvements
- ‘Denormals’ performance improvement for ‘soloing’ glitches


- Notes:
- Projects created in 2.0 beta are not compatible with version 2.0rc1 onwards.

## Version 2.0.b12
### Features/Improvements
- Control rate and audio thread count are saved in the service configuration (config.json)
- Visibility properties, rendering enable and framerate settings are saved/loaded after shutdown/startup
- OSC output enable settings are saved/loaded after shutdown/startup.
- New colour scheme & UI tweaks
- Speaker setup shapeReverb sound object with spatial reverb
- Particle box frame visualisation
- Selection visualisation improvement
- Improvements regarding audio settings change crashes

### Fixes
- b11 ‘ghost audio’ fix
- Fix for occasional crash when changing the particle count while the granulator is enabled

- Known issue:
- Selecting any spiral shape or python shape will crash or freeze the engine
- The Engine sometimes crashes when audio settings are changed, this is still under investigation, it appears to be extra sensitive to input drivers with large amounts of channels (>50)  and a buffer size of 2048


## Version 2.0.b11
### Features/Improvements
- Control thread: running control calculations separately from the main/rendering thread
- Display channel index in speaker label
- Effects optimization
- Tilted “triangularPrism” panning shape
- Splash screen at start-up
- Real-time adjustable OSC output IP & port
- Sound objects are black by default  
- Group levels now go up to +12dB

### Fixes
- The multi-window performance issue should be fixed by the control thread update (the rendering still shows lag, but it does not affect the sound anymore)
- Granulator density overload fix
- Grid settings fix
- Panning values visualisation order fix
- Shape transformations work with particle counts > 512.

## Version 2.0.b10
### Features/Improvements
- Allows at run-time changes of speaker setup and routing via GUI
- Recording channels start counting at 1 instead of at 0
- Removed redundant input effects of ‘source’ sound objects
- Show I/O per sound object in monitor side panel
- Allow disabling sound objects over OSC
- Added option to show/hide displaying panning values

### Fixes
- Monitor header spacing fix
- “Ghost audio” fix (preventing erroneous additional connections to effect chain outputs)
- Particle halos now take into account ‘direct loudness’.
- Two-digit input channel layout fix
- Grid names automatically converted to lowercase, to prevent osc address mismatches

- Notes:
- Rename the file names of old recordings (channel0.wav > channel1.wav, etc) in order to be able to play them back in Engine version 2.0.b10 and after.

- Known issue:
- The granulator is sometimes forever stuck on an extremely high density or 0 density. This is a known issue and will be fixed in 2.0.b11.

## Version 2.0.b9.2
### Fixes
- Halo border fix
- Gain scaling automatically enabled on start-up again
- Distance attenuation effect address fix
- Panning values display fix

## Version 2.0.b9.1

### Features/Improvements
- Fibonacci sphere shape particles are now three-dimensional, so the particles can be extruded
- OSC input to deselect sound object / select all
- Adjustable framerate cap
- Optimisation: particle VU meter levels are calculated in parallel
- Added option to automatically expose speaker amplitudes from the environment script

### Fixes
- Follow transformation fixed (prevent crash, use accurate names)
- DirectLoudness in stereo mode fix
- Don’t show effect labels of hidden sound objects.
- ‘Coupled’ headphones & multi-speaker master volume sliders fixed
- Particle rotation fix (bug since 2.0.b8 or b9)
- Path visualisation hotfix (bug since 2.0.b9)
## Version 2.0.b9
### Features/Improvements
- Larger floor grid
- Separately adjustable effect chains for sources and spaces
- Expose speaker amplitudes as osc output data
- Perception effects have been moved back to pre-mixdown.
- Lightning shaders improvements: option to add multiple lights, option to render the frame outline only using UV
- Attachable/detachable control panel window
- When searching in parameter/exposed data view, groups with no entries matching the search are automatically hidden.
- Grid & group settings are saved on shutdown.

### Fixes
- Path visualisation re-implemented
- Shape & particle rendering delay fixed
- Number of audio thread slider works again
- Napkin startup bug fix
- Speaker labels fix
- Windows & ImGUI fix
- Order of destruction bug in windows solved

- Maintenance/note:
- Effects are now added in the environment script and can be adjusted per sound object type.

- Known issues:
- When the Engine window is hidden behind another window, there is a dramatic drop in framerate, which is clearly audible. Therefore, make sure that at least a small part of the Engine window is always visible.

## Version 2.0.b8.3
### Features/Improvements
- OSC messages can be sent from environment script. After environment init now the message “/environment/init [source count] [space count] [group count] [source max particle count] [space max particle count]” is sent.
- Engine op time in seconds can be passed to uniform in particle visualization shaders
- Improved user experience for camera control with keys
## Version 2.0.b8.2
### Fixes
- Fix several issues that cause instabilities and crashes around real time editing in napkin, shaders, shape scripts, the environment script while the engine is running.

### Features/Improvements
- OSC message sent after the environment is initialized. Message and OSC port can be customized in napkin.
## Version 2.0.b8.1
### Features/Improvements
- Roughly feature parity with 2.0.b7
- A number of example particle visualizations to choose from or to adept: point, halo, cloud, box
## Version 2.0.b8
- Maintenance:
- Ported to NAP 0.4 with Vulkan rendering backend.
- Completely rewritten monitor from scratch for maintainability and more creative flow visualization coding
- Completely rewritten GUI from scratch for maintainability and finetuning layout and design
## Version 2.0.b7

### Features/Improvements
- Granulator buffer is cleared when disabled and enabled again
- Enable/disable monitor rendering in config file
- Activate/deactivate multi speaker setup in config file
- Specify default master volume in config file
- Save mastering parameters to config file
- Mastering can be defined in napkin and python per speaker type

### Fixes
- Headphones Panner enable/disable crash fix
- Crash on windows after running for 12 hours fix

- Maintenance:
- SpeakerSetup refactor & cleanup
- Remove parameter->connect() functions to stay in line with NAP master.




## Version 2.0.b6

### Features/Improvements
- Display license name in footer
- Display ‘beta version’ watermark
- Display error message when shape produces erroneous positions
- Binaural crossover threshold by default on 2.0 instead of 5.0
- OSC input for camera control
- Granulator transpose range to two octaves
- Source indices start counting from 1.

### Fixes
- Fibonacci sphere offset fix
- Hide paths when ‘entity’ visibility is disabled
- Keep particles visible when ‘entity’ visibility is disabled
- Marcel’s binaural fixes
- Movement orientation fixes (jitter at low speed / stopping path traversal)
- Fix crash on recording/playback in multispeaker setup
- Input distance effects won’t have an effect on external input anymore.
- Granulator multi-channel input fix (quick fix)

- Maintenance:
- HeadphonesPanner cleanup (stereo nodes & BinauralNode)
- Effect framework “recalculate” refactor (should also fix the doppler initial detune bug #244)
- Transformation/Effect/Shape/ShapeTransformation ctor/init refactor
- MeasurementComponent “recalculate” refactor

- Important note:
- Starting from version 2.0.b6, 4DSOUND 2.0 works with licenses different from those for 4DSOUND 1.X. Every user of 4DSOUND 2.0 should receive a new license key to be able to work with 4DSOUND Engine 2.0.b6.



## Version 2.0.b5 (Monom)
### Features/Improvements
- Engine won’t quit on escape
- Clearer logging of license status
- Indices for input level meters

### Fixes
- Binaural: explicit memory alignment (to prevent crashes)
- LPF, HPF & predelay are integrated into reverb effect

## Version 2.0.b4

### Features/Improvements
- Fibonacci sphere shape
- Stereo mode threshold & curve options
- Environment settings in separate file
- All HRTF options available in binaural panner
- Stability and performance boost of binaural panner (multi-threading, fixes for crashes, optimisation)
- Binaural: adjustable crossover threshold for better smoothing around listening point
- Stereo & binaural setup merged into a single “Headphones” panner, with a parameter to switch between stereo/binaural mode per sound object.
- Headphones output channels adjustable from config file
- All Headphones settings are saved/loaded at shutdown/startup.

### Fixes
- Stereo mode glitch fixes (on/off tick, smoothing)
- monitor/selectedEntity works with prefix instead of display name
- Removed binaural distance attenuation
- Don’t include perception effects in mixdown signal
- Point shape above circle shape in Shapes list

- Maintenance:
- SpatialSource refactor
- Deform & ParticleTransformation > ShapeTransformation refactor
- Important notes:
- The environment settings are now adjustable in the separate environmentSettings.py, instead of environment.py.
- “Deform” has been renamed to “ShapeTransformation”. Custom Deform/ShapeTransformation python scripts will need to be re-added in Napkin under SoundObject/ShapeComponent/ShapeTransformations. Also, OSC addresses have been changed (/deform/... >> /shapeTransformation/…, /particleTransformation/scale >> /shapeTransformation/particleScale, /particleTransformation/dimensions >> /shapeTransformation/particleDimensions, /particleTransformation/spatialDynamics/amount >> /shapeTransformation/particleSpatialDynamics/amount).


## Version 2.0.b3

### Features/Improvements

- All panners are now disabled when license key has not been activated
- Display ‘enter license key‘ at front panel if license has not been activated
- Added updated merkabah setups to data folder
- New square check boxes in line with m4l device design
- Selection panel categorised by ‘sources’ & ‘spaces’
- Zoom range increased & curved
- Globals & particle globals combined into ‘groups’
- Implemented visualisations for all shapes
- Updated spatial delay visualisation based on 1.X monitor algorithms and shape mesh
- Level meters for inputs & outputs per sound object in parameter panel
- Audio device GUI fixes
- Python interface improvements: work with transforms instead of just positions
- Python PrecalculatedShape, Shape, Deform, Transformation example scripts
- Blinking visualisation for OSC selection
- Axes helpers
- “Display particles as spheres” checkbox in UI
- Extended data panel with name, color, transform, particle count
- Spatial delay & reverb buffers clear before enabling
- Active channel count editable in audio device GUI (probably fixes external send bug)
- Cross convert vec2 & vec3 OSC messages between float and int
- SpatialPhaser memory allocation optimisation
- Granulator memory allocation optimisation


### Fixes

- WallsShape “distance” > “separation”
- Rename “Objects” to “Entities” in stats panel
- Remove “setup” / “decoder” from panner names
- Window title fix
- Wrap VU meters so they remain visible
- Particle size minimum fix
- Don’t show trail if object is not visible
- Monitor speaker VU meters fix
- “Samplerate” > “Sample Rate”
- Orthogonal camera option removed
- VesicaEye3D shape removed
- PushPull deform removed
- Panning amplitudes visualisation fix
- VU x Size fix
- Parameter panel initial postion/size fix
- Parameter panel value text wrapping fix
- Removed 3D monitor VU meter option
- Helix shape fix
- Better default input channels on Engine side (space1: 1, space2: 2, etc)
- “attract” > “bend”
- Spiral parameter name changes reverted
- Increase font size in monitor
- Allow audio output device failure at initialisation
- Reimplemented pyramid shape
- Cuboctahedron shape removed (see note)

- Maintenance:

- Upgrade to NAP 0.3 parameter system
- Renaming of AudioObject classes
- Important notes:
- Breaking change: Because of the parameter system change, and because python shape scripts now work with transforms instead of positions, old python shape scripts are not compatible with Engine version 2.0.3. Use the examples in the /scripts/examples as a reference when updating existing python scripts. Feel free to contact the development team if you encounter any difficulties in this.
- Using audio input devices with 64 or more input channels with buffer size 2048 currently causes crashes because of an issue outside our control (in portaudio).
- The faulty “cuboctahedron” shape has been removed. In order to know what is expected when we reimplement this shape, we would need clear documentation of the algorithm that should be used to distribute the particles (this can be in the form of pseudo code, a mathematical formula or drawings).
- The amount of memory allocation at startup should have decreased substantially. This could mean that the number of sound objects / spaces / particles can be increased at SSI. Something to try out!



## Version 2.0.2

- New features:

- Particle count as integer parameter, clipboard export functionality for M4L dropdown

### Fixes
- Startup crash fix
- Shutdown crash fix
- Only show input VU meters when audio input device is active
- Wavelength division by 0 fix.

- Maintenance:
- Extensive compile-time optimisation (cleanup includes, forward declares)
- Split mod_spatial module into separate modules (dsp, core, multispeaker, binaural, soundobject, implementations, gui)
## Version 2.0.1

- New features:
- • editable audio device settings
- • higher options for the /shape/shared/resolution parameter
- • 4dpan sphere shape
- • 4dpan tilted cube shape
- • doppler speed of sound parameter
- • doppler non-unisono compensating gain
- • input distance damping effect
- • walls shape visualisation improvement
- • adjustable stereo channels in stereo mode

### Fixes
- • pyramid shape fix
- • adaptive cube shape fix
- • stretchscale transformation fix
- • path visualisation fix
- • monitor z fighting artefacts fix
- • external input crash fix
- • box mesh fix

- Maintenance:
- • upgrade to NAP 0.3

- After starting and closing the Engine, a config.json file will be generated next to the executable. In here, in the 'SpatialServiceConfiguration', you can set the routing and the setup-file. Make sure to enter the path of the setup file like this: "SpeakerSetupFile": "setups/xxx.xml".

- Engine v2.0.1 works with M4L devices v0.2.16, except for the externalSend parameter. The externalSend parameter has become an integer instead of a string, which still needs to be updated in the M4L devices.

