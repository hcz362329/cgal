// Copyright (c) 2005  Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s): Simon Giraudot  <simon.giraudot@geometryfactory.com>

#ifndef CGAL_BSO_POLYGON_CONVERSIONS_H
#define CGAL_BSO_POLYGON_CONVERSIONS_H

#include <boost/range/join.hpp>
#include <boost/iterator/transform_iterator.hpp>

#include <CGAL/license/Boolean_set_operations_2.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/General_polygon_2.h>
#include <CGAL/General_polygon_with_holes_2.h>
#include <CGAL/Boolean_set_operations_2/Gps_default_traits.h>
#include <CGAL/Arr_polyline_traits_2.h>
#include <CGAL/Single.h>
#include <CGAL/Iterator_range.h>

namespace CGAL {

// Utility struct
template <typename Polygon>
struct Gps_polyline_traits {
  typedef typename Gps_default_traits<Polygon>::Arr_traits      Segment_traits;
  typedef Arr_polyline_traits_2<Segment_traits>                 Polyline_traits;
  typedef Gps_traits_2<Polyline_traits>                         Traits;
};

// Helper to map Polygon_2 -> General_polygon_2 / PWH_2 -> General_PWH_2
template <typename Polygon>
struct General_polygon_of_polygon;

//
template <typename Kernel, typename Container>
struct General_polygon_of_polygon<Polygon_2<Kernel, Container>> {
  using Pgn = Polygon_2<Kernel, Container>;
  using Polyline_traits = typename Gps_polyline_traits<Pgn>::Polyline_traits;
  using type = General_polygon_2<Polyline_traits>;
};

//
template <typename Kernel, typename Container>
struct General_polygon_of_polygon<Polygon_with_holes_2<Kernel, Container>> {
  using Pwh = Polygon_with_holes_2<Kernel, Container>;
  using Polyline_traits = typename Gps_polyline_traits<Pwh>::Polyline_traits;
  using General_pgm = General_polygon_2<Polyline_traits>;
  using type = General_polygon_with_holes_2<General_pgm>;
};

// Helper to detect if a type is Polygon_2 / PWH_2
template <typename InputIterator>
struct Is_Kernel_Polygon_2 { static constexpr bool value = false; };

template <typename Kernel>
struct Is_Kernel_Polygon_2<Polygon_2<Kernel>>
{ static constexpr bool value = true; };

template <typename Kernel>
struct Is_Kernel_Polygon_2<Polygon_with_holes_2<Kernel>>
{ static constexpr bool value = true; };

// Helper to enable/disable if InputIterator's value type is Polygon_2 / PWH_2
template <typename InputIterator>
using Enable_if_Polygon_2_iterator =
  typename std::enable_if
  <Is_Kernel_Polygon_2
   <typename std::iterator_traits<InputIterator>::value_type>::value>::type;

template <typename InputIterator>
using Disable_if_Polygon_2_iterator =
  typename std::enable_if
  <!Is_Kernel_Polygon_2
   <typename std::iterator_traits<InputIterator>::value_type>::value>::type;

// Convert Polygon_2 to General_polygon_2<Polyline_traits>
  template <typename Kernel, typename Container, typename ArrTraits>
General_polygon_2<ArrTraits>
convert_polygon(const Polygon_2<Kernel, Container>& polygon,
                const ArrTraits& traits)
{
  auto ctr = traits.construct_curve_2_object();
  if (polygon.is_empty()) return General_polygon_2<ArrTraits>();
  return ctr(boost::range::join(CGAL::make_range(polygon.vertices_begin(),
                                                 polygon.vertices_end()),
                                CGAL::make_single(*polygon.vertices_begin())));
}

// Convert Polygon_with_holes_2 to General_polygon_with_holes_2<Polyline_traits>
  template <typename Kernel, typename Container, typename ArrTraits>
General_polygon_with_holes_2<General_polygon_2<ArrTraits> >
convert_polygon(const Polygon_with_holes_2<Kernel, Container>& pwh,
                const ArrTraits& traits) {
  typedef General_polygon_2<ArrTraits>          General_pgn;
  typedef Polygon_2<Kernel, Container>          Pgn;
  auto converter = [&](const Pgn& pgn)->General_pgn {
    return convert_polygon(pgn, traits);
  };
  return General_polygon_with_holes_2<General_polygon_2<ArrTraits>>
    (convert_polygon(pwh.outer_boundary(), traits),
     boost::make_transform_iterator(pwh.holes().begin(), converter),
     boost::make_transform_iterator(pwh.holes().end(), converter));
}

// Convert General_polygon_2<Polyline_traits> to Polygon_2
template <typename Kernel, typename Container, typename ArrTraits>
Polygon_2<Kernel, Container>
convert_polygon_back(const General_polygon_2<ArrTraits>& gpgn) {
  Polygon_2<Kernel, Container> pgn;
  for (auto cit = gpgn.curves_begin(); cit != gpgn.curves_end(); ++cit) {
    // Skip last point, which is a duplication of the first point
    auto end = std::prev(cit->points_end());
    for (auto pit = cit->points_begin(); pit != end; ++pit) pgn.push_back(*pit);
  }
  return pgn;
}

// Convert General_polygon_with_holes_2<Polyline_traits> to Polygon_with_holes_2
template <typename Kernel, typename Container, typename ArrTraits>
Polygon_with_holes_2<Kernel, Container>
convert_polygon_back(const General_polygon_with_holes_2
                       <General_polygon_2<ArrTraits> >& gpwh)
{
  using Pgn = Polygon_2<Kernel, Container>;
  using General_pgn = General_polygon_2<ArrTraits>;
  auto converter = [](const General_pgn& gpgn)->Pgn {
    return convert_polygon_back<Kernel, Container>(gpgn);
  };
  return Polygon_with_holes_2<Kernel, Container>
    (convert_polygon_back<Kernel, Container>(gpwh.outer_boundary()),
     boost::make_transform_iterator(gpwh.holes().begin(), converter),
     boost::make_transform_iterator(gpwh.holes().end(), converter));
}

// Convert
// Polygon_2 to General_polygon_2<Polyline_traits>, or
// Polygon_with_holes_2 to General_polygon_with_holes_2<Polyline_traits>
template <typename InputIterator, typename Traits>
boost::transform_iterator
<std::function
 <typename General_polygon_of_polygon<typename std::iterator_traits
                                      <InputIterator>::value_type>::type
  (typename std::iterator_traits<InputIterator>::reference)>,
 InputIterator>
convert_polygon_iterator(InputIterator it, const Traits& traits)
{
  using Input_type = typename std::iterator_traits<InputIterator>::value_type;
  using Return_type = typename General_polygon_of_polygon<Input_type>::type;
  using Function_type = std::function<Return_type(Input_type)>;

  Function_type func =
    [&traits](const Input_type& p)->Return_type
    { return convert_polygon(p, traits); };

  return boost::transform_iterator<Function_type, InputIterator>(it, func);
}

// Polygon converter unary function
// Converts General_polygon_with_holes_2<Polyline_traits> to Polygon_with_holes_2
template <typename Kernel, typename Container, typename OutputIterator>
struct Polygon_converter {
  using Pgn = Polygon_2<Kernel, Container>;
  using Polyline_traits = typename Gps_polyline_traits<Pgn>::Polyline_traits;
  using Traits = typename Gps_polyline_traits<Pgn>::Traits;
  using General_polygon_with_holes = typename Traits::Polygon_with_holes_2;

  //! The output iterator.
  OutputIterator& m_output;

  //! Construct from an output iterator.
  Polygon_converter(OutputIterator& output) : m_output(output) {}

  // Convert and export to output iterator.
  void operator()(const  General_polygon_with_holes& gpwh) const
  { *m_output++ = convert_polygon_back<Kernel, Container>(gpwh); }
};

// Function output iterator wrapping OutputIterator with conversion to
// OutputIterator
template <typename Kernel, typename Container, typename OutputIterator>
struct Polygon_converter_output_iterator :
  boost::function_output_iterator<Polygon_converter
                                  <Kernel, Container, OutputIterator> >
{
  using Converter = Polygon_converter<Kernel, Container, OutputIterator>;
  using Base = boost::function_output_iterator<Converter>;

  OutputIterator& output;
  Polygon_converter_output_iterator(OutputIterator& output) :
    Base(output),
    output(output)
  {}

  operator OutputIterator() const { return output; }
};

// Converts General_polygon2<Polyline_traits> to Polygon_2
// (indirection with Polygon_2)
template <typename OutputIterator, typename Kernel, typename Container>
Polygon_converter_output_iterator<Kernel, Container, OutputIterator>
convert_polygon_back(OutputIterator& output,
                     const Polygon_2<Kernel, Container>&)
{
  return Polygon_converter_output_iterator
    <Kernel, Container, OutputIterator>(output);
}

// Converts General_polygon_with_holes_2<Polyline_traits> to Polygon_with_holes_2
// (indirection with Polygon_with_holes_2)
template <typename OutputIterator, typename Kernel, typename Container>
Polygon_converter_output_iterator<Kernel, Container, OutputIterator>
convert_polygon_back(OutputIterator& output,
                     const Polygon_with_holes_2<Kernel, Container>&)
{
  return Polygon_converter_output_iterator
    <Kernel, Container, OutputIterator>(output);
}

// Utility for checking if polygon remains the same after being
// converted and back
template <typename Kernel, typename Container>
Polygon_2<Kernel, Container>
test_conversion(const Polygon_2<Kernel, Container>& polygon)
{
  typedef Polygon_2<Kernel, Container>                  Pgn;
  typedef Polygon_with_holes_2<Kernel, Container>       Pgn_with_holes;
  typedef typename Gps_default_traits<Pgn>::Arr_traits  Segment_traits;
  typedef Arr_polyline_traits_2<Segment_traits>         Polyline_traits;
  typedef Gps_traits_2<Polyline_traits>                 Traits;

  using General_polygon = typename Traits::Polygon_2;
  using General_polygon_with_holes = typename Traits::Polygon_with_holes_2;
  Polyline_traits traits;

  General_polygon polygon2 = convert_polygon(polygon, traits);
  return convert_polygon_back<Kernel, Container>(polygon2);
  // General_polygon_with_holes polygon3(polygon2);

  // Pgn_with_holes out;
  // Oneset_iterator<Pgn_with_holes> iterator(out);
  // auto converter = convert_polygon_back(iterator, polygon3);
  // *converter++ = polygon3;

  // return out.outer_boundary();
}

}

#endif // CGAL_BSO_POLYGON_CONVERSIONS_H
