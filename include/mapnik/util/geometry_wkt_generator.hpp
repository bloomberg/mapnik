/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/

//$Id$

#ifndef MAPNIK_GEOMETRY_WKT_GENERATOR_HPP
#define MAPNIK_GEOMETRY_WKT_GENERATOR_HPP

// mapnik
#include <mapnik/global.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/util/vertex_iterator.hpp>
#include <mapnik/util/container_adapter.hpp>

// boost
#include <boost/tuple/tuple.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/home/phoenix/statement/if.hpp>
#include <boost/fusion/include/boost_tuple.hpp>


//#define BOOST_SPIRIT_USE_PHOENIX_V3 1

namespace mapnik { namespace util {

namespace karma = boost::spirit::karma;
namespace phoenix = boost::phoenix;

struct get_type
{
    template <typename T>
    struct result { typedef int type; };

    int operator() (geometry_type const& geom) const
    {
        return (int)geom.type();
    }
};

struct get_first
{
    template <typename T>
    struct result { typedef geometry_type::value_type const type; };

    geometry_type::value_type const operator() (geometry_type const& geom) const
    {
        geometry_type::value_type coord;
        boost::get<0>(coord) = geom.get_vertex(0,&boost::get<1>(coord),&boost::get<2>(coord));
        return coord;
    }
};

template <typename T>
struct coordinate_policy : karma::real_policies<T>
{
    typedef boost::spirit::karma::real_policies<T> base_type;
    static int floatfield(T n) { return base_type::fmtflags::fixed; }
    static unsigned precision(T n) { return 6u ;}
};


template <typename OutputIterator>
struct wkt_generator : 
        karma::grammar<OutputIterator, geometry_type const& ()>
{
    
    wkt_generator()
        : wkt_generator::base_type(wkt)
    {
        using boost::spirit::karma::uint_;
        using boost::spirit::karma::_val;
        using boost::spirit::karma::_1;
        using boost::spirit::karma::lit;
        using boost::spirit::karma::_a;
        using boost::spirit::karma::_r1;
        using boost::spirit::karma::eps;
        
        wkt = point | linestring | polygon 
            ;
        
        point = &uint_(mapnik::Point)[_1 = _type(_val)] 
            << lit("Point(") << point_coord [_1 = _first(_val)] << lit(')')
            ;
        
        linestring = &uint_(mapnik::LineString)[_1 = _type(_val)] 
            << lit("LineString(") 
            << coords 
            << lit(')')
            ;
        
        polygon = &uint_(mapnik::Polygon)[_1 = _type(_val)]
            << lit("Polygon(")
            << coords2
            << lit("))")            
            ;
        
        point_coord = &uint_ << coord_type << lit(' ') << coord_type 
            ;
        
        polygon_coord %= ( &uint_(mapnik::SEG_MOVETO) << eps[_r1 += 1] 
                   << karma::string[ if_ (_r1 > 1) [_1 = "),("]
                                     .else_[_1 = "("] ] | &uint_ << ",") 
            << coord_type 
            << lit(' ') 
            << coord_type
            ; 
        
        coords2 %= *polygon_coord(_a)
            ;

        coords = point_coord % lit(',')
            ;
        
    }
    // rules
    karma::rule<OutputIterator, geometry_type const& ()> wkt; 
    karma::rule<OutputIterator, geometry_type const& ()> point;
    karma::rule<OutputIterator, geometry_type const& ()> linestring;
    karma::rule<OutputIterator, geometry_type const& ()> polygon;

    karma::rule<OutputIterator, geometry_type const& ()> coords;
    karma::rule<OutputIterator, karma::locals<unsigned>, geometry_type const& ()> coords2;
    karma::rule<OutputIterator, geometry_type::value_type ()> point_coord;
    karma::rule<OutputIterator, geometry_type::value_type const& (unsigned& )> polygon_coord;
    
    // phoenix functions
    phoenix::function<get_type > _type;
    phoenix::function<get_first> _first;
    //
    karma::real_generator<double, coordinate_policy<double> > coord_type;
    
};

}}


#endif // MAPNIK_GEOMETRY_WKT_GENERATOR_HPP
