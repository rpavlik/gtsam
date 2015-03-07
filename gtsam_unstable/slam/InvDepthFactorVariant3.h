
/**
 * @file InvDepthFactorVariant3.h
 * @brief Inverse Depth Factor based on Civera09tro, Montiel06rss.
 * Landmarks are parameterized as (theta,phi,rho). The factor involves
 * two poses and a landmark. The first pose is the reference frame
 * from which (theta, phi, rho) is measured.
 * @author Chris Beall, Stephen Williams
 */

#pragma once

#include <gtsam/nonlinear/NonlinearFactor.h>
#include <gtsam/geometry/PinholeCamera.h>
#include <gtsam/geometry/Cal3_S2.h>
#include <gtsam/geometry/Pose3.h>
#include <gtsam/geometry/Point2.h>
#include <gtsam/base/LieVector.h>
#include <gtsam/base/numericalDerivative.h>

namespace gtsam {

/**
 * Binary factor representing the first visual measurement using an inverse-depth parameterization
 */
class InvDepthFactorVariant3a: public NoiseModelFactor2<Pose3, LieVector> {
protected:

  // Keep a copy of measurement and calibration for I/O
  Point2 measured_;        ///< 2D measurement
  Cal3_S2::shared_ptr K_;  ///< shared pointer to calibration object

public:

  /// shorthand for base class type
  typedef NoiseModelFactor2<Pose3, LieVector> Base;

  /// shorthand for this class
  typedef InvDepthFactorVariant3a This;

  /// shorthand for a smart pointer to a factor
  typedef boost::shared_ptr<This> shared_ptr;

  /// Default constructor
  InvDepthFactorVariant3a() : K_(new Cal3_S2(444, 555, 666, 777, 888)) {}

  /**
   * Constructor
   * TODO: Mark argument order standard (keys, measurement, parameters)
   * @param measured is the 2 dimensional location of point in image (the measurement)
   * @param model is the standard deviation
   * @param poseKey is the index of the camera pose
   * @param pointKey is the index of the landmark
   * @param invDepthKey is the index of inverse depth
   * @param K shared pointer to the constant calibration
   */
  InvDepthFactorVariant3a(const Key poseKey, const Key landmarkKey,
      const Point2& measured, const Cal3_S2::shared_ptr& K, const SharedNoiseModel& model) :
        Base(model, poseKey, landmarkKey), measured_(measured), K_(K) {}

  /** Virtual destructor */
  virtual ~InvDepthFactorVariant3a() {}

  /**
   * print
   * @param s optional string naming the factor
   * @param keyFormatter optional formatter useful for printing Symbols
   */
  void print(const std::string& s = "InvDepthFactorVariant3a",
      const gtsam::KeyFormatter& keyFormatter = gtsam::DefaultKeyFormatter) const {
    Base::print(s, keyFormatter);
    measured_.print(s + ".z");
  }

  /// equals
  virtual bool equals(const gtsam::NonlinearFactor& p, double tol = 1e-9) const {
    const This *e = dynamic_cast<const This*>(&p);
    return e
        && Base::equals(p, tol)
        && this->measured_.equals(e->measured_, tol)
        && this->K_->equals(*e->K_, tol);
  }

  Vector inverseDepthError(const Pose3& pose, const LieVector& landmark) const {
    try {
      // Calculate the 3D coordinates of the landmark in the Pose frame
      double theta = landmark(0), phi = landmark(1), rho = landmark(2);
      Point3 pose_P_landmark(cos(phi)*sin(theta)/rho, sin(phi)/rho, cos(phi)*cos(theta)/rho);
      // Convert the landmark to world coordinates
      Point3 world_P_landmark = pose.transform_from(pose_P_landmark);
      // Project landmark into Pose2
      PinholeCamera<Cal3_S2> camera(pose, *K_);
      gtsam::Point2 reprojectionError(camera.project(world_P_landmark) - measured_);
      return reprojectionError.vector();
    } catch( CheiralityException& e) {
      std::cout << e.what()
          << ": Inverse Depth Landmark [" << DefaultKeyFormatter(this->key1()) << "," << DefaultKeyFormatter(this->key2()) << "]"
          << " moved behind camera [" << DefaultKeyFormatter(this->key1()) << "]"
          << std::endl;
      return gtsam::ones(2) * 2.0 * K_->fx();
    }
    return gtsam::Vector_(1, 0.0);
  }

  /// Evaluate error h(x)-z and optionally derivatives
  Vector evaluateError(const Pose3& pose, const LieVector& landmark,
      boost::optional<gtsam::Matrix&> H1=boost::none,
      boost::optional<gtsam::Matrix&> H2=boost::none) const {

    if(H1) {
      (*H1) = numericalDerivative11<Pose3>(boost::bind(&InvDepthFactorVariant3a::inverseDepthError, this, _1, landmark), pose);
    }
    if(H2) {
      (*H2) = numericalDerivative11<LieVector>(boost::bind(&InvDepthFactorVariant3a::inverseDepthError, this, pose, _1), landmark);
    }

    return inverseDepthError(pose, landmark);
  }

  /** return the measurement */
  const gtsam::Point2& imagePoint() const {
    return measured_;
  }

  /** return the calibration object */
  const Cal3_S2::shared_ptr calibration() const {
    return K_;
  }

private:

  /// Serialization function
  friend class boost::serialization::access;
  template<class ARCHIVE>
  void serialize(ARCHIVE & ar, const unsigned int version) {
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Base);
    ar & BOOST_SERIALIZATION_NVP(measured_);
    ar & BOOST_SERIALIZATION_NVP(K_);
  }
};

/**
 * Ternary factor representing a visual measurement using an inverse-depth parameterization
 */
class InvDepthFactorVariant3b: public NoiseModelFactor3<Pose3, Pose3, LieVector> {
protected:

  // Keep a copy of measurement and calibration for I/O
  Point2 measured_;        ///< 2D measurement
  Cal3_S2::shared_ptr K_;  ///< shared pointer to calibration object

public:

  /// shorthand for base class type
  typedef NoiseModelFactor3<Pose3, Pose3, LieVector> Base;

  /// shorthand for this class
  typedef InvDepthFactorVariant3b This;

  /// shorthand for a smart pointer to a factor
  typedef boost::shared_ptr<This> shared_ptr;

  /// Default constructor
  InvDepthFactorVariant3b() : K_(new Cal3_S2(444, 555, 666, 777, 888)) {}

  /**
   * Constructor
   * TODO: Mark argument order standard (keys, measurement, parameters)
   * @param measured is the 2 dimensional location of point in image (the measurement)
   * @param model is the standard deviation
   * @param poseKey is the index of the camera pose
   * @param pointKey is the index of the landmark
   * @param invDepthKey is the index of inverse depth
   * @param K shared pointer to the constant calibration
   */
  InvDepthFactorVariant3b(const Key poseKey1, const Key poseKey2, const Key landmarkKey,
      const Point2& measured, const Cal3_S2::shared_ptr& K, const SharedNoiseModel& model) :
        Base(model, poseKey1, poseKey2, landmarkKey), measured_(measured), K_(K) {}

  /** Virtual destructor */
  virtual ~InvDepthFactorVariant3b() {}

  /**
   * print
   * @param s optional string naming the factor
   * @param keyFormatter optional formatter useful for printing Symbols
   */
  void print(const std::string& s = "InvDepthFactorVariant3",
      const gtsam::KeyFormatter& keyFormatter = gtsam::DefaultKeyFormatter) const {
    Base::print(s, keyFormatter);
    measured_.print(s + ".z");
  }

  /// equals
  virtual bool equals(const gtsam::NonlinearFactor& p, double tol = 1e-9) const {
    const This *e = dynamic_cast<const This*>(&p);
    return e
        && Base::equals(p, tol)
        && this->measured_.equals(e->measured_, tol)
        && this->K_->equals(*e->K_, tol);
  }

  Vector inverseDepthError(const Pose3& pose1, const Pose3& pose2, const LieVector& landmark) const {
    try {
      // Calculate the 3D coordinates of the landmark in the Pose1 frame
      double theta = landmark(0), phi = landmark(1), rho = landmark(2);
      Point3 pose1_P_landmark(cos(phi)*sin(theta)/rho, sin(phi)/rho, cos(phi)*cos(theta)/rho);
      // Convert the landmark to world coordinates
      Point3 world_P_landmark = pose1.transform_from(pose1_P_landmark);
      // Project landmark into Pose2
      PinholeCamera<Cal3_S2> camera(pose2, *K_);
      gtsam::Point2 reprojectionError(camera.project(world_P_landmark) - measured_);
      return reprojectionError.vector();
    } catch( CheiralityException& e) {
      std::cout << e.what()
          << ": Inverse Depth Landmark [" << DefaultKeyFormatter(this->key1()) << "," << DefaultKeyFormatter(this->key3()) << "]"
          << " moved behind camera " << DefaultKeyFormatter(this->key2())
          << std::endl;
      return gtsam::ones(2) * 2.0 * K_->fx();
    }
    return gtsam::Vector_(1, 0.0);
  }

  /// Evaluate error h(x)-z and optionally derivatives
  Vector evaluateError(const Pose3& pose1, const Pose3& pose2, const LieVector& landmark,
      boost::optional<gtsam::Matrix&> H1=boost::none,
      boost::optional<gtsam::Matrix&> H2=boost::none,
      boost::optional<gtsam::Matrix&> H3=boost::none) const {

    if(H1) {
      (*H1) = numericalDerivative11<Pose3>(boost::bind(&InvDepthFactorVariant3b::inverseDepthError, this, _1, pose2, landmark), pose1);
    }
    if(H2) {
      (*H2) = numericalDerivative11<Pose3>(boost::bind(&InvDepthFactorVariant3b::inverseDepthError, this, pose1, _1, landmark), pose2);
    }
    if(H3) {
      (*H3) = numericalDerivative11<LieVector>(boost::bind(&InvDepthFactorVariant3b::inverseDepthError, this, pose1, pose2, _1), landmark);
    }

    return inverseDepthError(pose1, pose2, landmark);
  }

  /** return the measurement */
  const gtsam::Point2& imagePoint() const {
    return measured_;
  }

  /** return the calibration object */
  const Cal3_S2::shared_ptr calibration() const {
    return K_;
  }

private:

  /// Serialization function
  friend class boost::serialization::access;
  template<class ARCHIVE>
  void serialize(ARCHIVE & ar, const unsigned int version) {
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Base);
    ar & BOOST_SERIALIZATION_NVP(measured_);
    ar & BOOST_SERIALIZATION_NVP(K_);
  }
};

} // \ namespace gtsam
