// Copyright 2015 The Edda Authors. All rights reserved.
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

#ifndef STRUCTURED_GRID_DATASET_H_
#define STRUCTURED_GRID_DATASET_H_

#include <vector>
#include <iostream>
#include <memory>
#include "grid.h"
#include "data_array.h"
#include <distributions/distribution.h>

namespace edda {




template <typename T>  // Return type of at_phys
class Dataset {
protected:
    Grid *pGrid;
    AbstractDataArray *pArray;
public:
    Dataset(Grid *pGrid, AbstractDataArray *pArray) {
        this->pGrid = pGrid;
        this->pArray = pArray;
    }
    ~Dataset() {
        delete pGrid;
        delete pArray;
    }

    Grid *getGrid() { return pGrid; }
    AbstractDataArray *getArray () {return pArray; }

    ReturnStatus at_phys(const VECTOR3 &pos, T &output) const {
        ReturnStatus r;
        switch (pGrid->getInterpType())
        {
        case InterpType::TRI_LERP:
        {

            PointInfo pinfo;
            pinfo.phyCoord = pos ;
            r = pGrid->phys_to_cell(pinfo);
            if (r != SUCCESS)
                return r;

            std::vector<int> vVertices;
            r = pGrid->getCellVertices(pinfo.inCell, vVertices);
            if (r != SUCCESS)
                return r;

            int i;
            std::vector<T> vData(vVertices.size());
            for (i=0; i<(int)vVertices.size(); i++)
                vData[i] = boost::any_cast<T> ( pArray->getItem(vVertices[i]) );


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

};


// Make creating a shared pointer of Dataset easier
template <typename T>  // Return type of at_phys
inline std::shared_ptr< Dataset<T> >
make_Dataset(Grid *pGrid, AbstractDataArray *pArray)
{
    return std::shared_ptr< Dataset<T> >(
                new Dataset<T> (pGrid, pArray) );
}

} // namespace edda

#endif // STRUCTURED_GRID_DATASET_H_
