#include "LayerExDraw.hpp"

extern void getPoints(const tTJSVariant &var, vector<PointF> &points);
extern void getRects(const tTJSVariant &var, vector<RectF> &rects);

Path::Path()
{
}

Path::~Path()
{
}

/**
 * ���݂̐}�`������Ɏ��̐}�`���J�n���܂�
 */
void
Path::startFigure()
{
    path.StartFigure();
}


/**
 * ���݂̐}�`����܂�
 */
void
Path::closeFigure()
{
    path.CloseFigure();
}

/**
 * �~�ʂ̕`��
 * @param x ������W
 * @param y ������W
 * @param width ����
 * @param height �c��
 * @param startAngle ���v�����~�ʊJ�n�ʒu
 * @param sweepAngle �`��p�x
 */
void
Path::drawArc(REAL x, REAL y, REAL width, REAL height, REAL startAngle, REAL sweepAngle)
{
	path.AddArc(x, y, width, height, startAngle, sweepAngle);
}

/**
 * �x�W�F�Ȑ��̕`��
 * @param app �A�s�A�����X
 * @param x1
 * @param y1
 * @param x2
 * @param y2
 * @param x3
 * @param y3
 * @param x4
 * @param y4
 */
void
Path::drawBezier(REAL x1, REAL y1, REAL x2, REAL y2, REAL x3, REAL y3, REAL x4, REAL y4)
{
	path.AddBezier(x1, y1, x2, y2, x3, y3, x4, y4);
}

/**
 * �A���x�W�F�Ȑ��̕`��
 * @param app �A�s�A�����X
 * @param points �_�̔z��
 */
void
Path::drawBeziers(tTJSVariant points)
{
	vector<PointF> ps;
	getPoints(points, ps);
	path.AddBeziers(&ps[0], (int)ps.size());
}

/**
 * Closed cardinal spline �̕`��
 * @param app �A�s�A�����X
 * @param points �_�̔z��
 */
void
Path::drawClosedCurve(tTJSVariant points)
{
	vector<PointF> ps;
	getPoints(points, ps);
	path.AddClosedCurve(&ps[0], (int)ps.size());
}

/**
 * Closed cardinal spline �̕`��
 * @param app �A�s�A�����X
 * @param points �_�̔z��
 * @pram tension tension
 */
void
Path::drawClosedCurve2(tTJSVariant points, REAL tension)
{
	vector<PointF> ps;
	getPoints(points, ps);
	path.AddClosedCurve(&ps[0], (int)ps.size(), tension);
}

/**
 * cardinal spline �̕`��
 * @param app �A�s�A�����X
 * @param points �_�̔z��
 */
void
Path::drawCurve(tTJSVariant points)
{
	vector<PointF> ps;
	getPoints(points, ps);
	path.AddCurve(&ps[0], (int)ps.size());
}

/**
 * cardinal spline �̕`��
 * @param app �A�s�A�����X
 * @param points �_�̔z��
 * @parma tension tension
 */
void
Path::drawCurve2(tTJSVariant points, REAL tension)
{
	vector<PointF> ps;
	getPoints(points, ps);
	path.AddCurve(&ps[0], (int)ps.size(), tension);
}

/**
 * cardinal spline �̕`��
 * @param app �A�s�A�����X
 * @param points �_�̔z��
 * @param offset
 * @param numberOfSegments
 * @param tension tension
 */
void
Path::drawCurve3(tTJSVariant points, int offset, int numberOfSegments, REAL tension)
{
	vector<PointF> ps;
	getPoints(points, ps);
	path.AddCurve(&ps[0], (int)ps.size(), offset, numberOfSegments, tension);
}

/**
 * �~���̕`��
 * @param x ������W
 * @param y ������W
 * @param width ����
 * @param height �c��
 * @param startAngle ���v�����~�ʊJ�n�ʒu
 * @param sweepAngle �`��p�x
 */
void
Path::drawPie(REAL x, REAL y, REAL width, REAL height, REAL startAngle, REAL sweepAngle)
{
	path.AddPie(x, y, width, height, startAngle, sweepAngle);
}

/**
 * �ȉ~�̕`��
 * @param app �A�s�A�����X
 * @param x
 * @param y
 * @param width
 * @param height
 */
void
Path::drawEllipse(REAL x, REAL y, REAL width, REAL height)
{
	path.AddEllipse(x, y, width, height);
}

/**
 * �����̕`��
 * @param app �A�s�A�����X
 * @param x1 �n�_X���W
 * @param y1 �n�_Y���W
 * @param x2 �I�_X���W
 * @param y2 �I�_Y���W
 */
void
Path::drawLine(REAL x1, REAL y1, REAL x2, REAL y2)
{
	path.AddLine(x1, y1, x2, y2);
}

/**
 * �A�������̕`��
 * @param app �A�s�A�����X
 * @param points �_�̔z��
 */
void
Path::drawLines(tTJSVariant points)
{
	vector<PointF> ps;
	getPoints(points, ps);
	path.AddLines(&ps[0], (int)ps.size());
}

/**
 * ���p�`�̕`��
 * @param app �A�s�A�����X
 * @param points �_�̔z��

 */
void
Path::drawPolygon(tTJSVariant points)
{
	vector<PointF> ps;
	getPoints(points, ps);
	path.AddPolygon(&ps[0], (int)ps.size());
}


/**
 * ��`�̕`��
 * @param app �A�s�A�����X
 * @param x
 * @param y
 * @param width
 * @param height
 */
void
Path::drawRectangle(REAL x, REAL y, REAL width, REAL height)
{
	RectF rect(x, y, width, height);
	path.AddRectangle(rect);
}

/**
 * ������`�̕`��
 * @param app �A�s�A�����X
 * @param rects ��`���̔z��
 */
void
Path::drawRectangles(tTJSVariant rects)
{
	vector<RectF> rs;
	getRects(rects, rs);
	path.AddRectangles(&rs[0], (int)rs.size());
}

/**
 * �p�X�̕`��
 * @param path �p�X���
 */
void
Path::drawPath(const Path *srcpath, bool connect)
{
	path.AddPath(&srcpath->path, connect ? TRUE : FALSE);
}
