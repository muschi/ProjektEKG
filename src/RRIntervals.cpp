#include "RRIntervals.h"
#include <iostream>
#include <algorithm>
#include <functional>
using namespace std;
namespace Ecg {
namespace AtrialFibr {

const static double longintervalpercentage = 1.15;
const static double shortintervalpercentage = 0.85;

RRIntervalMethod::RRIntervalMethod() {}

vector<double>
RRIntervalMethod::countRRInvervals(const vector<double> &RRtime) {
  vector<double> RRIntervals;
  transform(begin(RRtime) + 1, end(RRtime), begin(RRtime),
            back_inserter(RRIntervals), minus<double>());
  return RRIntervals;
}

vector<classification>
RRIntervalMethod::classifyIntervals(const vector<double> &RRIntervals) {
  vector<classification> classifiedIntervals;
  for (const auto &interval : RRIntervals) {
    if (interval >= longintervalpercentage * avarageInterval) {
      classifiedIntervals.push_back(Long);
    } else if (interval <= shortintervalpercentage * avarageInterval) {
      classifiedIntervals.push_back(Short);
    } else {
      classifiedIntervals.push_back(Regular);
    }
  }
  return classifiedIntervals;
}

void RRIntervalMethod::countAvarageInterval(const vector<double> &RRIntervals) {
  avarageInterval =
      accumulate(begin(RRIntervals), end(RRIntervals), 0.0, plus<double>()) /
      RRIntervals.size();
}

void RRIntervalMethod::countTransitions(
    const vector<classification> &classifiedIntervals) {

  markovTable.fill({ { 0.0, 0.0, 0.0 } });
  for (auto it = classifiedIntervals.begin();
       it != classifiedIntervals.end() - 1; ++it) {
    markovTable[*it][*(it + 1)] += 1;
  }
}
void RRIntervalMethod::normalizeMarkovTable() {}

std::array<double, 3> row(const Matrix3_3 &matrix, int n) { return matrix[n]; }

std::array<double, 3> col(const Matrix3_3 &matrix, int n) {
  array<double, 3> ans;
  transform(begin(matrix), end(matrix), begin(ans),
            [n](const array<double, 3> &row) { return row[n]; });
  return ans;
}

double entropy(const Matrix3_3 &matrix) {
  array<double, 3> H;
  for (int i = 0; i < 3; i++) {
    const auto _row = row(matrix, i);
    array<double, 3> multipliedByLog;
    transform(begin(_row), end(_row), begin(multipliedByLog),
              [](double x) { return x * log(x) / log(2); });
    H[i] = accumulate(begin(multipliedByLog), end(multipliedByLog), 0.0);
  }

  array<double, 3> P;
  for (int i = 0; i < 3; i++) {
    const auto column = col(matrix, i);
    P[i] = accumulate(begin(column), end(column), 0.0);
  }

  return -inner_product(begin(P), end(P), begin(H), 0.0);
}

double KLdivergence(const Matrix3_3 &transitionsMatrix,
                    const Matrix3_3 &patternMatrix) {
  double sum = 0;
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      sum += log(patternMatrix[i][j] / transitionsMatrix[i][j]) *
             patternMatrix[i][j];
    }
  }
  return sum;
}
double JKdivergence(const Matrix3_3 &transitionsMatrix,
                    const Matrix3_3 &patternMatrix) {
  Matrix3_3 M;
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; ++j) {
      M[i][j] = 0.5 * (patternMatrix[i][j] + transitionsMatrix[i][j]);
    }
  }
  return 0.5 *
         (KLdivergence(M, patternMatrix) + KLdivergence(M, transitionsMatrix));
}
}
}
