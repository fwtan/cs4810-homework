#include "rayScene.h"
#ifdef __APPLE__
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif
#include <math.h>


///////////////////////
// Ray-tracing stuff //
///////////////////////

Point3D RayScene::Reflect(Point3D v, Point3D n)
{
	v = v.unit();
	n = n.unit();
	double cos_theta = v.dot(n);

	if (cos_theta >= 0)
	{
		return Point3D();
	}

	return (v - n * 2 * (cos_theta)).unit();
}

int RayScene::Refract(Point3D v, Point3D n, double ir, Point3D& refract)
{
	v = -v.unit();
	n = n.unit();

	double cos_theta_i = v.dot(n);
	double theta_i = acos(cos_theta_i);
	double sin_theta_r = ir * sin(theta_i);

	if (sin_theta_r > 1)
	{
		return 0;
	}

	double theta_r = asin(sin_theta_r);

	refract = n * (ir * cos_theta_i - cos(theta_r)) - v * ir;
	refract = refract/refract.length();

	return 1;
}

Ray3D RayScene::GetRay(RayCamera* camera,int i,int j,int width,int height)
{
	double d = 1.f;

    double h = d * tan(0.5 * camera->heightAngle);
    double w = h * camera->aspectRatio;

    double u = -w + 2 * w * ((i + 0.5)/width);
    double v = -h + 2 * h * ((j + 0.5)/height);

    Point3D dir = camera->direction * d + camera->right * u + camera->up * v;
	return Ray3D(camera->position, dir.unit());
}

Point3D RayScene::GetColor(Ray3D ray, int rDepth, Point3D cLimit)
{
	RayIntersectionInfo iInfo;

	if (group->intersect(ray, iInfo) > 0)
	{
		Point3D intensity = iInfo.material->ambient + iInfo.material->emissive;
		for (int i = 0; i < lightNum; ++i)
		{
			Point3D fShadow = lights[i]->transparency(iInfo, group, cLimit);
			intensity += lights[i]->getDiffuse(ray.position, iInfo).mult(fShadow);
			intensity += lights[i]->getSpecular(ray.position, iInfo).mult(fShadow);
		}

		double factor = 0.001;
		BoundingBox3D bbox = group->bBox;
		Point3D p0 = bbox.p[0];
		Point3D p1 = bbox.p[1];
		double dist = (p1 - p0).length();


		Ray3D reflected_ray;
		reflected_ray.direction = Reflect(ray.direction, iInfo.normal);
		if (reflected_ray.direction.length() > 0.5)
		{
			reflected_ray.position = iInfo.iCoordinate + reflected_ray.direction * (dist * factor);
			Point3D Ks = iInfo.material->specular;
			if (rDepth > 0 && Ks[0] > cLimit[0] && Ks[1] > cLimit[1] && Ks[2] > cLimit[2])
			{
				Point3D reflected_color = GetColor(reflected_ray, rDepth - 1, cLimit.div(Ks));
				intensity += Ks.mult(reflected_color);
			}
		}

		Ray3D refracted_ray;
		if (Refract(ray.direction, iInfo.normal, 1.0/iInfo.material->refind, refracted_ray.direction) > 0)
		{
			refracted_ray.position = iInfo.iCoordinate + refracted_ray.direction * (dist * factor);

			Point3D Kt = iInfo.material->transparent;
			if (rDepth > 0 && Kt[0] > cLimit[0] && Kt[1] > cLimit[1] && Kt[2] > cLimit[2])
			{
				Point3D refracted_color = GetColor(refracted_ray, rDepth - 1, cLimit.div(Kt));
				intensity += Kt.mult(refracted_color);
			}
		}

		return intensity;
	}
	return Point3D();
}

//////////////////
// OpenGL stuff //
//////////////////
void RayMaterial::drawOpenGL(void){
}
void RayTexture::setUpOpenGL(void){
}
