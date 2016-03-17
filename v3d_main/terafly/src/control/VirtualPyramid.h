#ifndef VIRTUALPYRAMID_H
#define VIRTUALPYRAMID_H

#include "CPlugin.h"
#include "VirtualVolume.h"

// Virtual Pyramid class
// - a container for virtual pyramid layers
// - a container and access point for caching
class teramanager::VirtualPyramid
{
    private:

        // object members
        iim::VirtualVolume*                 highresVol;     // highest-res (unconverted) volume
        std::string                         highresPath;    // highest-res (unconverted) volume path
        std::vector< itm::VirtualPyramidLayer* > pyramid;   // virtual pyramid layers, from highest-res to lowest-res
        std::string                         localPath;      // where local files should be stored

        // disable default constructor
        VirtualPyramid(){}

        // object utility methods
        void init() throw (iim::IOException, iom::exception, itm::RuntimeException);    // init volume and metadata


    public:

        // constructor 1
        VirtualPyramid(
                std::string _highresPath,                   // highest-res (unconverted) volume path
                int reduction_factor,                       // pyramid reduction factor (i.e. divide by reduction_factor along all axes for all layers)
                float lower_bound = 100,                    // lower bound (in MVoxels) for the lowest-res pyramid image (i.e. divide by reduction_factor until the lowest-res has size <= lower_bound)
                iim::VirtualVolume* _highresVol = 0)        // highest-res (unconverted) volume, if null will be instantiated on-the-fly
        throw (iim::IOException, iom::exception, itm::RuntimeException);


        // constructor 2
        VirtualPyramid(
                std::string _highresPath,                   // highest-res (unconverted) volume path
                std::vector< xyz<int> > reduction_factors,  // pyramid reduction factors (i.e. divide by reduction_factors[i].x along X for layer i)
                iim::VirtualVolume* _highresVol = 0)        // highest-res (unconverted) volume, if null will be instantiated on-the-fly
        throw (iim::IOException, iom::exception, itm::RuntimeException);


        // destructor
        ~VirtualPyramid() throw(iim::IOException);


        // GET methods
        std::vector<iim::VirtualVolume*> getPyramid(){
            std::vector<iim::VirtualVolume*> tmp(pyramid.begin(), pyramid.end());
            std::reverse(tmp.begin(), tmp.end());
            return tmp;
        }

        friend class VirtualPyramidLayer;
};


// Virtual Pyramid Layer class
// - a wrapper built on the highest-res image to intercept its load methods
// - inherits from VirtualVolume, which makes using a Virtual Pyramid Image transparent to the client
// - communicates with the parent (VirtualPyramid) image cache
//     - the load() methods invoked on the highest-res layer  SEND    image data TO   the cache
//     - the load() methods invoked on the lower-res   layers RECEIVE image data FROM the cache
class teramanager::VirtualPyramidLayer : public iim::VirtualVolume
{
    private:

        // object members
        itm::VirtualPyramid*    parent;             // container
        int                     level;              // pyramid level (0 = highest-res, the coarser the resolution the higher)
        itm::xyz<int>           reductionFactor;    // pyramid reduction factor relative to the highest-res image
        itm::HyperGridCache*    cache;              // dedicated cache for this layer

        // disable default constructor
        VirtualPyramidLayer(){}

    public:

        // constructor
        VirtualPyramidLayer(
                int _level,                         // pyramid level (0 = highest-res, the coarser the resolution the higher)
                VirtualPyramid* _parent,            // container
                xyz<int> _reduction_factor)         // reduction factor relative to the highest-res image
        throw (iim::IOException, itm::RuntimeException);

        // deconstructor
        virtual ~VirtualPyramidLayer() throw (iim::IOException);

        // GET methods
        VirtualPyramid* getPyramid(){return parent;}

        // inherited pure virtual methods, to implement
        virtual void initChannels ( ) throw (iim::IOException);
        virtual iim::real32 *loadSubvolume_to_real32(int V0=-1,int V1=-1, int H0=-1, int H1=-1, int D0=-1, int D1=-1)  throw (iim::IOException);
        virtual iim::uint8 *loadSubvolume_to_UINT8(int V0=-1,int V1=-1, int H0=-1, int H1=-1, int D0=-1, int D1=-1, int *channels=0, int ret_type=iim::DEF_IMG_DEPTH) throw (iim::IOException);
        virtual float getVXL_1(){return VXL_H;}
        virtual float getVXL_2(){return VXL_V;}
        virtual float getVXL_3(){return VXL_D;}
        virtual iim::axis getAXS_1(){return parent->highresVol->getAXS_1();}
        virtual iim::axis getAXS_2(){return parent->highresVol->getAXS_2();}
        virtual iim::axis getAXS_3(){return parent->highresVol->getAXS_3();}
        virtual std::string getPrintableFormat(){return std::string("VirtualPyramid on ") + parent->highresVol->getPrintableFormat();}

        // return true if 'highresVol' downsampled by 'reduction_factor' generates a 0-sized image
        static bool isEmpty(iim::VirtualVolume* highresVol, xyz<int> _reduction_factor);
};


// Hyper Grid Cache class
// - designed to cache image data during visualization
// - send/receive image data to/from client
// - store cached data permanently on the disk
// - supports up to 5D images
class teramanager::HyperGridCache
{
    private:

        // object members
        class CacheBlock;                       // forward-declaration
        std::string path;                       // where cache files are stored / have to be stored
        CacheBlock ******hypergrid;             // 5D array of <CacheBlock>, follows T-C-Z-Y-Z order
        xyzct<size_t> block_dim;                // desired dimensions of each <CacheBlock>
        size_t nX, nY, nZ, nC, nT;              // hypergrid dimension along X, Y, Z, C (channel), and T (time)
        size_t dimX, dimY, dimZ, dimC, dimT;    // image space dimensions along X, Y, Z, C (channel), and T (time)

        // object methods
        HyperGridCache(){}                      // disable default constructor
        void load() throw (iim::IOException, itm::RuntimeException);   // load from disk
        void save() throw (iim::IOException, itm::RuntimeException);   // save to disk
        void init() throw (iim::IOException, itm::RuntimeException);   // init persistency files

    public:

        // constructor 1
        HyperGridCache(
                std::string _path,                                                      // where cache files are stored / have to be stored
                xyzct<size_t> image_dim,                                                // image dimensions along X, Y, Z, C (channel), and T (time)
                xyzct<size_t> _block_dim = xyzct<size_t>(256,256,256,inf<size_t>(),inf<size_t>())) // hypergrid block dimensions along X, Y, Z, C (channel), and T (time)
        throw (iim::IOException, itm::RuntimeException);

        // destructor
        ~HyperGridCache() throw (iim::IOException);


        // read data from the cache (downsampling on-the-fly supported)
        image_5D<uint8>                                         // <image data, image data size> output
        readData(
                xyzct<size_t> start,                            // start coordinate in the current 5D image space
                xyzct<size_t> end,                              // end coordinate in the current 5D image space
                xyz<int> downsamplingFactor = xyz<int>(1,1,1))  // downsampling factors along X,Y and Z
        throw (iim::IOException);


        // put data into the cache (downsampling on-the-fly supported)
        void putData(
                const image_5D<uint8>,                          // image data array, follows T-C-Z-Y-Z order
                xyzct<size_t> shift,                            // shift relative to (0,0,0,0,0)
                xyz<int> downsamplingFactor = xyz<int>(1,1,1))  // downsampling factors along X,Y and Z
        throw (iim::IOException);


        // class members
        static float maximumSizeGB;             // maximum size (in Gigabytes) for this Cache (default: 1 GB)


    private:

        // Cache Block nested class
        // - stores individual portions (blocks) of cached data and the corresponding visit counts
        class CacheBlock
        {
            private:

                // object members
                HyperGridCache* parent;                 // container
                xyzct<size_t> origin;                   // origin coordinate of the block in the image 5D (xyz+channel+time) space, start at (0,0,0,0,0)
                image_5D<uint8> imdata;                 // cached image data
                xyzct<size_t> idx;                      // 5D index in the parent hypergrid
                std::string path;                       // path of file where this block is stored
                int visits;                             // #times this block has been visited (for both loading and storing of image data)

                // object utility methods
                CacheBlock(){}                          // disable default constructor
                void load() throw (iim::IOException, iom::exception);   // load from disk
                void save() throw (iim::IOException, iom::exception);   // save to disk


            public:


                // contructor 1
                CacheBlock(
                        HyperGridCache* _parent,
                        xyzct<size_t> _origin,          // origin coordinate of the block in the image 5D (xyz+channel+time) space, start at (0,0,0,0,0)
                        xyzct<size_t> _dims,            // dimensions of the block
                        xyzct<size_t> _index)           // 5D index in the parent hypergrid
                throw (iim::IOException);

                // destructor
                ~CacheBlock() throw (iim::IOException);

                // GET and SET methods
                xyzct<size_t> getOrigin(){return origin;}
                xyzct<size_t> getDims(){return imdata.dims;}
                int getVisits(){return visits;}
                void setVisits(int _visits){visits=_visits;}
                uint8* getImageDataPtr(){return imdata.data;}
        };
};



#endif // VIRTUALPYRAMID_H
