// Copyright 2015 The Edda Authors. All rights reserved.
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

#ifndef DATASET_H_
#define DATASET_H_

#include <vector>
#include <iostream>
#include <memory>
#include "dataset/distr_array.h"
#include "curvilinear_grid.h"
#include "distributions/variant.h"
#include "grid.h"
namespace edda {

///
/// \brief Holds all information of a dataset, which includes: 1) Geometry and 2) Data array.
///
/// You may use make_Dataset() to create a Dataset with shorter codes.
/// \param T The return type of at_phys() and at_comp()
///
template <typename T>
class Dataset {
protected:
    Grid *pGrid;
    DistrArray *pArray;
public:
    Dataset(Grid *pGrid, DistrArray *pArray) {
      this->pGrid = pGrid;
      this->pArray = pArray;
    }
    ~Dataset() {
      if (pGrid)
        delete pGrid;
      if (pArray)
        delete pArray;
    }

    Grid *getGrid() { return pGrid; }
    DistrArray *getArray () {return pArray; }

    ///
    /// \brief Get the dimension of the cartesian-grid data.
    ///
    /// Non-cartesian grid data are currently not supported.
    ///
    int *getDimension() {
      CartesianGrid *cartesianGrid = dynamic_cast<CartesianGrid *>(pGrid) ;
      if (!cartesianGrid)
        throw NotImplementedException();
      return cartesianGrid->getDimension();
    }

    ///
    /// \brief Return the interpolated data at a given position
    /// \param pos Position to query
    /// \param[out] The query result.  Valid only when the function returns SUCCESS
    /// \return SUCCESS if success, or OUT_OF_BOUND if the position is out of boundary.
    ///
    ReturnStatus at_phys(const VECTOR3 &pos, T &output) const {
        ReturnStatus r;
        switch (pGrid->getInterpType())
        {
        case TRI_LERP:
        {
            PointInfo pinfo;
            pinfo.phyCoord = pos ;
            r = pGrid->phys_to_cell(pinfo);
            if (r != SUCCESS)
                return r;

            std::vector<size_t> vVertices;
            r = pGrid->getCellVertices(pinfo.inCell, vVertices);
            if (r != SUCCESS)
                return r;

            int i;
            std::vector<T> vData(vVertices.size());
            for (i=0; i<vVertices.size(); i++)
            {

              getSample(vVertices[i], vData[i]);
            }

            output = triLerp(vData[0], vData[1], vData[2], vData[3],
                             vData[4], vData[5], vData[6], vData[7],
                             pinfo.interpolant.getData());
            break;
        }
        default:
            throw NotImplementedException();
            break;
        }

        return SUCCESS;
    }


    ///
    /// \brief Return the data in the computational space.
    /// \param i,j,k The coordinates in the computational space.
    /// \return The query result.
    /// Only for structured grids.
    /// If <i,j,k> is out of boundary, the function will throw OutOfBoundException.
    ///
    const T at_comp(int i, int j, int k) const {
      ReturnStatus r;
      CartesianGrid *cartesianGrid = dynamic_cast<CartesianGrid *>(pGrid) ;

      if (cartesianGrid) {
        size_t idx;
        r = cartesianGrid->getIndex(i,j,k, idx);
        if (r!=SUCCESS)
          throw OutOfBoundException();

        //return boost::any_cast<const T&>( pArray->getItem(idx) );
        T data;
        getSample(idx, data);
        return data;
      }

      // TODO for other grid types
      throw NotImplementedException();
    }

    ///
    /// \brief Return the distribution in the computational space.
    /// \param i,j,k The coordinates in the computational space.
    /// \return The query result.
    /// Only for structured grids.
    /// If <i,j,k> is out of boundary, the function will throw OutOfBoundException.
    ///
    const dist::Variant at_comp_distr(int i, int j, int k) const {
      ReturnStatus r;
      CartesianGrid *cartesianGrid = dynamic_cast<CartesianGrid *>(pGrid) ;

      if (cartesianGrid) {
        size_t idx;
        r = cartesianGrid->getIndex(i,j,k, idx);
        if (r!=SUCCESS)
          throw OutOfBoundException();

        return pArray->getDistr(idx);
      }

      // TODO for other grid types
      throw NotImplementedException();
    }

    ///
    /// \brief Return the distribution vector in the computational space.
    /// \param i,j,k The coordinates in the computational space.
    /// \return The query result, in std::vector.
    /// Only for structured grids.
    /// If <i,j,k> is out of boundary, the function will throw OutOfBoundException.
    ///
    const std::vector<dist::Variant>  at_comp_distr_vector(int i, int j, int k) const {
      ReturnStatus r;
      CartesianGrid *cartesianGrid = dynamic_cast<CartesianGrid *>(pGrid) ;

      if (cartesianGrid) {
        size_t idx;
        r = cartesianGrid->getIndex(i,j,k, idx);
        if (r!=SUCCESS)
          throw OutOfBoundException();

        return pArray->getDistrVector(idx);
      }

      // TODO for other grid types
      throw NotImplementedException();
    }

protected:
    void getSample(int idx, Real &data) const {
      data = pArray->getScalar(idx) ;
    }

    template <int N>
    void getSample(int idx, Vector<Real, N> &data) const {
      std::vector<Real> varvec =  pArray->getVector(idx) ;
      assert(N == varvec.size());
      for (int c=0; c<N; c++) {
         data[c] = varvec[c];
      }
    }

};


///
/// \brief Create a shared pointer of Dataset
///
template <typename T>  // Return type of at_phys
inline std::shared_ptr< Dataset<T> >
make_Dataset(Grid *pGrid, DistrArray *pArray)
{
    return std::shared_ptr< Dataset<T> >(
                new Dataset<T> (pGrid, pArray) );
}

} // namespace edda

#endif // STRUCTURED_GRID_DATASET_H_
