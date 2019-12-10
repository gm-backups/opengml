rm -r release/ > /dev/null

_make="make"

if [ "$#" -neq 1 ]; then
    _make="$1"
    shift
fi

cmake -DRELEASE=ON .
$_make -j 8

if [ $? == 0 ]; then
    mkdir release/
    cp -r demo/ release/demo
    cp ogm.exe release/
    cp *.dll release/
    cp README.md release
    cp LICENSE release

    # open source licensing obligations.
    external="external"
    mkdir release/${external}

    # rstartree
    cp -r external/include/rstartree release/${external}/rstartree
    cp external/include/rstartree/LICENSE release/LICENSE_rstartree

    # yaml
    cp external/include/yaml/LICENSE release/LICENSE_yaml

    # pugixml
    cp external/pugixml/LICENCE.md release/LICENSE_pugixml

    # nlohmann
    cp external/include/nlohmann/LICENCE.MIT release/LICENSE_nlohmann

    # rectpack2D
    cp external/include/rectpack2D/LICENSE.md release/LICENSE_rectpack2d

    # simpleini
    cp external/include/simpleini/LICENCE.txt release/LICENSE_simpleini

    # ThreadPool
    cp external/include/ThreadPool_zlib_license.txt release/LICENCE_ThreadPool

    # catch2 not included in release.
fi
