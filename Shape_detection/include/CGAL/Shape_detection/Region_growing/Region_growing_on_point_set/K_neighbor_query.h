// Copyright (c) 2018 INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Florent Lafarge, Simon Giraudot, Thien Hoang, Dmitry Anisimov
//

#ifndef CGAL_SHAPE_DETECTION_REGION_GROWING_POINT_SET_K_NEIGHBOR_QUERY_H
#define CGAL_SHAPE_DETECTION_REGION_GROWING_POINT_SET_K_NEIGHBOR_QUERY_H

#include <CGAL/license/Shape_detection.h>

// STL includes.
#include <typeinfo>
#include <type_traits>

// Boost includes.
#include <CGAL/boost/graph/named_params_helper.h>
#include <CGAL/boost/iterator/counting_iterator.hpp>
#include <CGAL/boost/graph/Named_function_parameters.h>

// CGAL includes.
#include <CGAL/Kd_tree.h>
#include <CGAL/Splitters.h>
#include <CGAL/assertions.h>
#include <CGAL/Search_traits_2.h>
#include <CGAL/Search_traits_3.h>
#include <CGAL/Search_traits_adapter.h>
#include <CGAL/Orthogonal_k_neighbor_search.h>

// Internal includes.
#include <CGAL/Shape_detection/Region_growing/internal/property_map.h>

namespace CGAL {
namespace Shape_detection {
namespace Point_set {

  /*!
    \ingroup PkgShapeDetectionRGOnPoints

    \brief K nearest neighbors search in a set of `Kernel::Point_2` or
    `Kernel::Point_3`.

    This class returns the K nearest neighbors of a query point in a point set.

    \tparam GeomTraits
    a model of `Kernel`

    \tparam InputRange
    a model of `ConstRange` whose iterator type is `RandomAccessIterator`

    \tparam PointMap
    a model of `LValuePropertyMap` whose key type is the value type of the input
    range and value type is `Kernel::Point_2` or `Kernel::Point_3`

    \cgalModels `NeighborQuery`
  */
  template<
  typename GeomTraits,
  typename InputRange,
  typename PointMap>
  class K_neighbor_query {

  public:
    /// \cond SKIP_IN_MANUAL
    using Traits = GeomTraits;
    using Input_range = InputRange;
    using Point_map = PointMap;
    using Point_type = typename Point_map::value_type;
    /// \endcond

  private:
    using Index_to_point_map =
    internal::Item_property_map<Input_range, Point_map>;

    using Search_base = typename std::conditional<
      std::is_same<typename Traits::Point_2, Point_type>::value,
      CGAL::Search_traits_2<Traits>,
      CGAL::Search_traits_3<Traits> >::type;

    using Search_traits =
    CGAL::Search_traits_adapter<std::size_t, Index_to_point_map, Search_base>;

    using Distance =
    CGAL::Distance_adapter<
      std::size_t,
      Index_to_point_map,
      CGAL::Euclidean_distance<Search_base> >;

    using Splitter =
    CGAL::Sliding_midpoint<Search_traits>;

    using Search_tree =
      CGAL::Kd_tree<Search_traits, Splitter, CGAL::Tag_true, CGAL::Tag_true>;

    using Neighbor_search =
    CGAL::Orthogonal_k_neighbor_search<
      Search_traits,
      Distance,
      Splitter,
      Search_tree>;

    using Tree =
    typename Neighbor_search::Tree;

  public:
    /// \name Initialization
    /// @{

    /*!
      \brief initializes a Kd-tree with input points.

      \param input_range
      an instance of `InputRange` with 2D or 3D points

      \param k
      the number of returned neighbors per each query point. %Default is 12.

      \param point_map
      an instance of `PointMap` that maps an item from `input_range`
      to `Kernel::Point_2` or to `Kernel::Point_3`

      \pre `input_range.size() > 0`
      \pre `k > 0`
    */
    template<typename NamedParameters>
    K_neighbor_query(
      const InputRange& input_range,
      const NamedParameters& np,
      const PointMap point_map = PointMap()) :
    m_input_range(input_range),
    m_point_map(point_map),
    m_index_to_point_map(m_input_range, m_point_map),
    m_distance(m_index_to_point_map),
    m_tree(
      boost::counting_iterator<std::size_t>(0),
      boost::counting_iterator<std::size_t>(m_input_range.size()),
      Splitter(),
      Search_traits(m_index_to_point_map)) {

      CGAL_precondition(input_range.size() > 0);
      const std::size_t k = parameters::choose_parameter(
        parameters::get_parameter(np, internal_np::neighbor_radius), 12);
      CGAL_precondition(k > 0);
      m_number_of_neighbors = k;
      m_tree.build();
    }

    /// @}

    /// \name Access
    /// @{

    /*!
      \brief implements `NeighborQuery::operator()()`.

      This operator finds indices of the `k` closest points to the point with
      the index `query_index` using a Kd-tree. These indices are returned in `neighbors`.

      \param query_index
      index of the query point

      \param neighbors
      indices of points, which are neighbors of the query point

      \pre `query_index >= 0 && query_index < input_range.size()`
    */
    void operator()(
      const std::size_t query_index,
      std::vector<std::size_t>& neighbors) const {

      neighbors.clear();
      CGAL_precondition(query_index < m_input_range.size());
      Neighbor_search neighbor_search(
        m_tree,
        get(m_index_to_point_map, query_index),
        static_cast<unsigned int>(m_number_of_neighbors),
        0, true, m_distance);
      for (auto it = neighbor_search.begin(); it != neighbor_search.end(); ++it)
        neighbors.push_back(it->first);
    }

    /// @}

    /// \cond SKIP_IN_MANUAL
    // A property map that can be used to access points when
    // iterating over the indices counting items in the input range.
    const Index_to_point_map& index_to_point_map() const {
      return m_index_to_point_map;
    }
    /// \endcond

  private:
    const Input_range& m_input_range;
    const Point_map m_point_map;
    const Index_to_point_map m_index_to_point_map;

    std::size_t m_number_of_neighbors;
    Distance m_distance;
    Tree m_tree;
  };

} // namespace Point_set
} // namespace Shape_detection
} // namespace CGAL

#endif // CGAL_SHAPE_DETECTION_REGION_GROWING_POINT_SET_K_NEIGHBOR_QUERY_H
