# PrivacyProject
The source code of this project has been developed as part of a PhD thesis about Visual Privacy Protection.
Specifically, it consists in the protection of individual's privacy appearing in images and videos. For this
purpose, a protection scheme is proposed that is based on visualisation models in order to have several privacy
levels, where each one provides a different balance between privacy protection and intelligibility. In turn,
this protection scheme enables individuals to specify their privacy preferences concerning given circumstances
and a desired privacy level. This way, whenever such circumstances are detected, the preferred privacy level is
automatically applied. As it can be guessed, the proposed scheme relies on the context in order to do this.

### Modules
So, the source code included as part of this project is related to image modification and context detection. It is organised in several modules that are described below:

* [CoreLib] - A shared library that includes all of the interfaces and types used. It also contains code that wrap the OpenNI/NiTE library and load several datasets for action recognition and person re-identification.

* [PrivacyFilterLib] - A shared library that includes code related to image modification in order to protect privacy. Visualisation models are implemented here as GLSL shaders for OpenGL ES 2.0.

* [DatasetBrowser] - A tool used to explore several datasets like MSRAction3D, MSRDailyActivity and HuDaAct. This tool may need some love in order to support all of the datasets of CoreLib :).

* [DatasetParser] - Supported datasets are processed so as to create a XML description. This tool carries out that task.

* [PrivacyFilters] - This tool is able of opening an ONI file or a Kinect-like device suppported by OpenNI/NiTE. Then, the visualisation models implemented in PrivacyFilterLib can be triggered on demand.

* [PrivacyEditor] - As it name says, this tool is a privacy editor so it is able of modifying a still image by means of the visualisation models provided.

* [PersonReid] - As part of the context detection, recognising the identity of individuals is essential in order to retrieve their privacy profiles. This module provides three features for person re-identification.


License
----

[Apache License, version 2.0](http://www.apache.org/licenses/LICENSE-2.0)

