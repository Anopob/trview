#pragma once 
#include <fstream>
#include <vector>

namespace trlevel
{
    template<typename T> static T read ( std::istream& file )
    {
        T value;
        read<T> ( file, value );
        return value;
    }

    template<typename T> static void read ( std::istream& file, T& value )
    {
        file.read ( reinterpret_cast<char*>(&value), sizeof ( value ) );
    }

    template < typename DataType, typename SizeType >
    std::vector<DataType> read_vector ( std::istream& file, SizeType size )
    {
        std::vector<DataType> data ( size );
        for ( SizeType i = 0; i < size; ++i )
        {
            read<DataType> ( file, data [i] );
        }
        return data;
    }

    template < typename SizeType, typename DataType >
    std::vector<DataType> read_vector ( std::istream& file )
    {
        auto size = read<SizeType> ( file );
        return read_vector<DataType, SizeType> ( file, size );
    }

    std::vector<uint8_t> read_compressed ( std::istream& file )
    {
        auto uncompressed_size = read<uint32_t> ( file );
        auto compressed_size = read<uint32_t> ( file );
        auto compressed = read_vector<uint8_t> ( file, compressed_size );
        std::vector<uint8_t> uncompressed_data ( uncompressed_size );

        z_stream stream;
        memset ( &stream, 0, sizeof ( stream ) );
        int result = inflateInit ( &stream );
        // Exception...
        stream.avail_in = compressed_size;
        stream.next_in = &compressed [0];
        stream.avail_out = uncompressed_size;
        stream.next_out = &uncompressed_data [0];
        result = inflate ( &stream, Z_NO_FLUSH );
        inflateEnd ( &stream );

        return uncompressed_data;
    }

    template < typename DataType >
    std::vector<DataType> read_vector_compressed ( std::istream& file, uint32_t elements )
    {
        auto uncompressed_data = read_compressed ( file );
        std::string data ( reinterpret_cast<char*>(&uncompressed_data [0]), uncompressed_data.size () );
        std::istringstream data_stream ( data, std::ios::binary );
        data_stream.exceptions ( std::ifstream::failbit | std::ifstream::badbit | std::ifstream::eofbit );
        return read_vector<DataType> ( data_stream, elements );
    }

    void skip ( std::istream& file, uint32_t size )
    {
        file.seekg ( size, std::ios::cur );
    }

    void skip_xela ( std::istream& file )
    {
        skip ( file, 4 );
    }
 
    std::vector<tr4_mesh_face4> read_psx_tr2_face4 ( std::istream& file, std::streamoff off )
    {
        // skip padding
        file.seekg ( 2, SEEK_CUR );

        int16_t count = read<int16_t> ( file );

        std::vector<tr4_mesh_face4> vec;
        vec.resize ( count );

        if ( count )
        {
            for ( int i = 0; i < count; ++i )
            {
                vec [i].texture = read<uint16_t> ( file );
            }
            if ( ( file.tellg () - off ) % 4 )
            {
                file.seekg ( 2, SEEK_CUR );
            }
            for ( int i = 0; i < count; ++i )
            {
                for ( int v = 0; v < 4; ++v )
                {
                    vec [i].vertices [v] = read<uint16_t> ( file ) >> 2;
                }
            }
        }

        return vec;
    }
    
    std::vector<tr4_mesh_face3> read_psx_tr2_face3 ( std::istream& file, std::streamoff off )
    {
        int16_t count = read<int16_t> ( file );

        // skip padding
        file.seekg ( 2, SEEK_CUR );

        std::vector<tr4_mesh_face3> vec;
        vec.resize ( count );

        if ( count )
        {
            for ( int i = 0; i < count; ++i )
            {
                vec [i].texture = read<uint16_t> ( file );

                for ( int v = 0; v < 3; ++v )
                {
                    vec [i].vertices [v] = read<uint16_t> ( file ) >> 2;
                }
            }
        }
        return vec;
    }
}