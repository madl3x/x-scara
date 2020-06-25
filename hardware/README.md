Sets
===

Hardware for X-SCARA is organized in sets. Current sets are **frame**, **arm** and **accessories**. 

For printing and assembly instructions of:

 * frame -> [go here](frame/README.md)
 * arm -> [go here](arm/README.md)
 * accessories -> [go here](accessories/README.md)

Versioning
===

Being work in progress, a versioning scheme will is necessary to identify which parts are part of a working set and which not. The versioning scheme used will be:

    partname_major_minor_revision.STL

Hardware parts with different major and minor values should not be mounted together if they are part of the same set (e.g. arm of frame). Different revisions for the same part will work fine together with the previously printed parts from the same set. 

Different minor versions between different sets will work well together as well. For example, a frame of version **1.1.x** will work well with an arm of version **1.2.x**. But, an arm component of version **1.2.x** will not work with a previously printed arm component of version **1.1.x**. 

When sets will become incompatible (e.g. frame with arm) the major version will be incremented (e.g. frame version **1.x.x** will not math any arm components with version **2.x.x**).

Bill of materials
===
Bill of materials is available per set. Each set will publish the list of parts required to build it.
