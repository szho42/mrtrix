/*
    Copyright 2008 Brain Research Institute, Melbourne, Australia

    Written by J-Donald Tournier, 27/06/08.

    This file is part of MRtrix.

    MRtrix is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    MRtrix is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with MRtrix.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "app.h"
#include "image/position.h"

using namespace std; 
using namespace MR; 

SET_VERSION_DEFAULT;


DESCRIPTION = {
 "multiply images",
  NULL
};

ARGUMENTS = {
  Argument ("input", "input image", "input image to be multiplied.", true, true).type_image_in (),
  Argument ("output", "output image", "the output image.").type_image_out (),
  Argument::End
};


OPTIONS = { Option::End };


EXECUTE {
  int num_images = argument.size() - 1;
  std::vector<RefPtr<Image::Object> > in_obj (num_images);

  in_obj[0] = argument[0].get_image();

  Image::Header header (*in_obj[0]);
  header.data_type = DataType::Float32;

  for (int i = 1; i < num_images; i++) {
    in_obj[i] = argument[i].get_image();
    if (in_obj[i]->is_complex()) header.data_type = DataType::CFloat32;

    if (in_obj[i]->ndim() > header.ndim()) header.axes.set_ndim (in_obj[i]->ndim());

    for (int n = 0; n < header.ndim(); n++) { 
      if (header.dim(n) != in_obj[i]->dim(n)) {
        if (header.dim(n) < 2) header.axes.copy (n, in_obj[i]->header().axes, n);
        else if (in_obj[i]->dim(n) > 1) 
          throw Exception ("dimension mismatch between input files");
      }
    }
  }


  Image::Position out (*argument.back().get_image (header));

  ProgressBar::init (num_images*out.voxel_count(), "multiplying...");

  for (int i = 0; i < num_images; i++) {

    out.zero();
    Image::Position in (*in_obj[i]);

    do {

      for (int n = 0; n < in.ndim(); n++)
        in.set (n, in.dim(n) > 1 ? out[n] : 0);

      if (out.is_complex()) {
        Math::ComplexNumber<float> c (1.0, 0.0);
        if (i) c = out.Z();
        if (in.is_complex()) c *= in.Z();
        out.Z (c);
      } 
      else {
        float val = i ? out.value() : 1.0;
        out.value (val * in.value());
      }

      ProgressBar::inc();

    } while (out++);

  }

  ProgressBar::done();

}
