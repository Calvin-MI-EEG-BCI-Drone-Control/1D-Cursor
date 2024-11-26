import matplotlib.pyplot as plt
import numpy as np
# From NeurotechX https://github.com/jdpigeon/bci-workshop

class DataPlotter():
    """
    Class for creating and updating a line plot.
    """

    def __init__(self, nbPoints, chNames, fs=None, title=None):
        """Initialize the figure."""

        self.nbPoints = nbPoints
        self.chNames = chNames
        self.nbCh = len(self.chNames)

        self.fs = 1 if fs is None else fs
        self.figTitle = '' if title is None else title

        data = np.empty((self.nbPoints, 1))*np.nan
        self.t = np.arange(data.shape[0])/float(self.fs)

        # Create offset parameters for plotting multiple signals
        self.yAxisRange = 100
        self.chRange = self.yAxisRange/float(self.nbCh)
        self.offsets = np.round((np.arange(self.nbCh)+0.5)*(self.chRange))

        # Create the figure and axis
        plt.ion()
        self.fig, self.ax = plt.subplots()
        self.ax.set_yticks(self.offsets)
        self.ax.set_yticklabels(self.chNames)

        # Initialize the figure
        self.ax.set_title(self.figTitle)

        self.chLinesDict = {}
        for i, chName in enumerate(self.chNames):
            self.chLinesDict[chName], = self.ax.plot(
                    self.t, data+self.offsets[i], label=chName)

        self.ax.set_xlabel('Time')
        self.ax.set_ylim([0, self.yAxisRange])
        self.ax.set_xlim([np.min(self.t), np.max(self.t)])

        plt.show()

    def update_plot(self, data):
        """ Update the plot """

        data = data - np.mean(data, axis=0)
        std_data = np.std(data, axis=0)
        std_data[np.where(std_data == 0)] = 1
        data = data/std_data*self.chRange/5.0

        for i, chName in enumerate(self.chNames):
            self.chLinesDict[chName].set_ydata(data[:, i] + self.offsets[i])

        self.fig.canvas.draw()

    def clear(self):
        """ Clear the figure """

        blankData = np.empty((self.nbPoints, 1))*np.nan

        for i, chName in enumerate(self.chNames):
            self.chLinesDict[chName].set_ydata(blankData)

        self.fig.canvas.draw()

    def close(self):
        """ Close the figure """

        plt.close(self.fig)


def plot_classifier_training(clf, X, y, features_to_plot=[0, 1]):
    """Visualize the decision boundary of a classifier.

    Args:
        clf (sklearn object): trained classifier
        X (numpy.ndarray): data to visualize the decision boundary for
        y (numpy.ndarray): labels for X

    Keyword Args:
        features_to_plot (list): indices of the two features to use for
            plotting

    Inspired from: http://scikit-learn.org/stable/auto_examples/tree/plot_iris.html
    """

    plot_colors = "bry"
    plot_step = 0.02
    n_classes = len(np.unique(y))

    x_min = np.min(X[:, 1])-1
    x_max = np.max(X[:, 1])+1
    y_min = np.min(X[:, 0])-1
    y_max = np.max(X[:, 0])+1

    xx, yy = np.meshgrid(np.arange(x_min, x_max, plot_step),
                         np.arange(y_min, y_max, plot_step))

    Z = clf.predict(np.c_[xx.ravel(), yy.ravel()])
    Z = Z.reshape(xx.shape)

    fig, ax = plt.subplots()
    ax.contourf(xx, yy, Z, cmap=plt.cm.Paired, alpha=0.5)

    # Plot the training points
    for i, color in zip(range(n_classes), plot_colors):
        idx = np.where(y == i)
        ax.scatter(X[idx, 0], X[idx, 1], c=color, cmap=plt.cm.Paired)

    plt.axis('tight')