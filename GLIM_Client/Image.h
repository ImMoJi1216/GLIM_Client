#pragma once
class Image
{
public:
    CvvImage() {}
    virtual ~CvvImage() { release(); }
    void release() { if (image) cvReleaseImage(&image); }
    operator IplImage* () { return image; }
    IplImage* operator->() { return image; }
    const IplImage* operator->() const { return image; }
    void create(int w, int h, int d) { release(); image = cvCreateImage(cvSize(w, h), d, 1); }
    void create(cv::Mat& m) { release(); image = new IplImage(m); }
private:
    IplImage* image;
};

