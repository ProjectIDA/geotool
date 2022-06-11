#ifndef _FK3D_H
#define _FK3D_H

#include "motif++/Frame.h"
#include "motif++/MotifDecs.h"
#include "FK.h"
extern "C" {
#include "ggl.h"
}

/** @defgroup libgfk3d plugin FK3D
 */

namespace libgfk {

class FK;

/** FK3D window.
 *  @ingroup libgfk3d
 */
class Fk3D : public Frame
{

    public:
	Fk3D(const char *, Component *, FK *fk);
	Fk3D(const char *, Component *, FK *fk, FKType);
	~Fk3D(void);

	void set3DScale(int i, double x_min, double x_max, double y_min,
		double y_max, float fk_min, float fk_max);
	void setXYScale(int i, double x_min, double x_max, double y_min,
		double y_max);
	void showOneFK(int i, int nx, double *slow_x, int ny, double *slow_y,
		float *fk);
	void setBandLabels(void);
	void draw(void);
	void drawFK(double slow_limit, long band);

    protected:

	FKType type;
	FK *fk;

	// File menu
	Button *close_button;

	// View menu
	Toggle *grid_toggle, *normalize_toggle;

	// Help menu
	Button *fk_help_button;

	Label *label1, *label2, *label3, *label4;
	Form *form1, *form2, *form3, *form4;

	Scale *scale;

	struct gglWidget_s *ggl[4];

	void createInterface(FKType);
	void createSingleInterface(void);
	void createMultiInterface(void);

	void actionPerformed(ActionEvent *action_event);

    private:

};

} // namespace libgfk

#endif
