from DataStream import *
from Model import *
from DataPlotter import *
import time
import os

# much of this code is directly from or based on https://github.com/jdpigeon/bci-workshop

def record_data_by_time(data_stream, training_length):
        '''
        Record data based on a certain amount of time.
        training_length is a time to record samples (in seconds)
        '''
        end_time = time.time() + training_length
        samples = []
        while time.time() < end_time:
            new_data = data_stream.get_data()
            if new_data is not None: samples.append(new_data)
        return samples

def record_data_by_samples(data_stream, training_length):
    '''
    Record data based on a certain number of samples to be recorded.
    training_length is a count of how many samples to collect.
    '''
    sample_num = 0
    samples = []
    while sample_num < training_length:
        new_data = data_stream.get_data()
        if new_data is not None: 
            samples.append(new_data)
            sample_num += 1 # only increment loop if a sample is added to the array
    return samples
    
def record_training_data(data_stream, training_length, record_type="time"):
    '''
    @param data_stream: a DataStream where signal data is gotten from
    @param training_length: the length of time to collect training data for each condition
    '''
    

    clear_console = lambda : os.system('cls' if os.name == 'nt' else 'clear')
    
    clear_console()
    input("Press enter to start recording training data.")
    clear_console()

    print("IMAGINE MOVING YOUR LEFT HAND...")
    lh_array = record_data_by_time(data_stream, training_length) if record_type == "time" else record_data_by_samples(data_stream, training_length)
    clear_console()

    input("Press enter to record the next condition.")
    clear_console()

    print("IMAGINE MOVING YOUR RIGHT HAND...")
    rh_array = record_data_by_time(data_stream, training_length) if record_type == "time" else record_data_by_samples(data_stream, training_length)
    clear_console()

    print("Training Data Finished Recording\n")
    return lh_array, rh_array

def record_testing_data(data_stream, training_length, record_type="time"):
    return record_data_by_time(data_stream, training_length) if record_type == "time" else record_data_by_samples(data_stream, training_length)

# create a numpy array of raw data
# df = pd.read_csv("/kaggle/input/eeg-motor-imagery-bciciv-2a/BCICIV_2a_all_patients.csv")
# df = pd.read_csv("/kaggle/input/BCICIV_2a_all_patients.csv") # bugged storage location of input datasets...
# df.shape

# grouped_arrays = {key: group.values for key, group in df.groupby('label')}
# grouped_arrays.keys()

# # see the length of the epochs included in the dataset (currently not used)
# df.groupby('epoch').size()
# df[(df['label'] == 'left')].groupby('epoch').size()

# get data for left/right hand, and drop the metadata columns
# lh_raw = grouped_arrays['left'][:, 4:]
# rh_raw = grouped_arrays['right'][:, 4:]

# split into epochs
# epoch_size = 100 # 1 * "fs", assuming fs == 100 == nominal sampling rate
# overlap = 80 # .8 * "fs"
# lh_epochs = epoch(lh_raw, epoch_size, overlap)
# rh_epochs = epoch(rh_raw, epoch_size, overlap)
# print(lh_epochs.shape)
# print(rh_epochs.shape)

# feature extraction
# lh_feat_matrix = compute_feature_matrix(lh_epochs, 100)
# rh_feat_matrix = compute_feature_matrix(rh_epochs, 100)
# print(lh_feat_matrix.shape)

# split into training/testing
# from sklearn.model_selection import train_test_split
# X_train_l, X_test_l, y_train_l, y_test_l = train_test_split(lh_feat_matrix, np.zeros(lh_feat_matrix.shape), test_size=0.2)
# X_train_r, X_test_r, y_train_r, y_test_r = train_test_split(rh_feat_matrix, np.ones(rh_feat_matrix.shape), test_size=0.2)

# train a classifier (usually gives a warning)
# [classifier, mu_ft, std_ft] = train_classifier(
#             X_train_l, X_train_r)

# make predictions for each class
# x0 = (X_test_l - mu_ft) / std_ft
# y_pred0 = classifier.predict(x0)
# x1 = (X_test_r - mu_ft) / std_ft
# y_pred1 = classifier.predict(x1)

# concat results
# y_pred = np.concatenate((y_pred0, y_pred1))
# y_true = np.concatenate((np.zeros(y_pred0.shape), np.ones(y_pred1.shape)))

# metrics
# from sklearn.metrics import mean_squared_error, accuracy_score
# print(f'mse: {mean_squared_error(y_true, y_pred)}')
# print(f'acc: {accuracy_score(y_true, y_pred)}')

# first run: .288 mse, .712 acc

training_time_length = 30

print("Preparing Data Stream...")
stream = DataStream()

# clear junk data (A delay between when the hardware is started and when meaningful data is being transmitted)
temp = stream.get_data() 
# if stream.get_data() returns none or an array of all 0s
while temp == None or all(value == 0.0 for value in temp):
    # returns None if the function is called faster than the data is published/received
    temp = stream.get_data()

lh_raw, rh_raw = record_training_data(stream, training_time_length)
print(np.array(lh_raw).shape)
print(np.array(rh_raw).shape)
print()

# Divide data into epochs
NOMINAL_SAMPLING_RATE = 100
epoch_length = 1
epoch_size = epoch_length * NOMINAL_SAMPLING_RATE # 1 * sampling_rate
# handle cases where very few samples are collected
smallest_sample = min(len(lh_raw), len(rh_raw))
if (smallest_sample) < epoch_size:
    epoch_size = smallest_sample
    print(f"WARNING: epoch_size < nominal sampling rate of {NOMINAL_SAMPLING_RATE}")    # may cause an actual error, so not exactly a "warning"
overlap = .8 * NOMINAL_SAMPLING_RATE # .8 * sampling_rate
lh_epochs = epoch(lh_raw, epoch_size, overlap)
rh_epochs = epoch(rh_raw, epoch_size, overlap)
print(lh_epochs.shape)
print(rh_epochs.shape)
print()

# Compute Features and Train Classifier
# feature extraction
lh_feat_matrix = compute_feature_matrix(lh_epochs, epoch_size)
rh_feat_matrix = compute_feature_matrix(rh_epochs, epoch_size)
print(lh_feat_matrix.shape)
print(rh_feat_matrix.shape)
print()

# train a classifier (usually gives a WARNING)
[classifier, mu_ft, std_ft] = train_classifier(
            lh_feat_matrix, rh_feat_matrix)

# Initialize the buffers for storing raw EEG and decisions
eeg_buffer = np.zeros((int(epoch_size), stream.num_channels))
filter_state = None  # for use with the notch filter
decision_buffer = np.zeros((30, 1))

plotter_decision = DataPlotter(30, ['Decision'])

# The try/except structure allows to quit the while loop by aborting the
# script with <Ctrl-C>
print('Press Ctrl-C in the console to break the while loop.')

try:
    while True:

        """ 3.1 ACQUIRE DATA """
        # Obtain EEG data from the LSL stream
        shift_length = epoch_length - overlap
        new_eeg_sample = record_testing_data(stream, epoch_size, "number")

        # Only keep the channel we're interested in
        # ch_data = np.array(eeg_data)[:, index_channel]

        # Update EEG buffer
        # eeg_buffer, filter_state = update_buffer(
        #         eeg_buffer, np.array(new_eeg_sample), notch=True,
        #         filter_state=filter_state)

        # """ 3.2 COMPUTE FEATURES AND CLASSIFY """
        # Get newest samples from the buffer
        # data_epoch = get_last_data(eeg_buffer,
        #                                 epoch_size)

        #TESTING
        data_epoch = np.array(new_eeg_sample)

        # Compute features
        feat_vector = compute_feature_vector(data_epoch, epoch_size)
        y_hat = test_classifier(classifier,
                                        feat_vector.reshape(1, -1), mu_ft,
                                        std_ft)
        print(f"prediction: {y_hat}")

        decision_buffer, _ = update_buffer(decision_buffer,
                                                np.reshape(y_hat, (-1, 1)))

        """ 3.3 VISUALIZE THE DECISIONS """
        plotter_decision.update_plot(decision_buffer)
        plt.pause(0.00001)

    
        # alternative:
        #  """ 3.1 ACQUIRE DATA """
        # # Obtain EEG data from the LSL stream
        # shift_length = epoch_length - overlap
        # new_eeg_sample = record_testing_data(stream, shift_length * NOMINAL_SAMPLING_RATE, "number")

        # # Only keep the channel we're interested in
        # # ch_data = np.array(eeg_data)[:, index_channel]

        # # Update EEG buffer
        # eeg_buffer, filter_state = update_buffer(
        #         eeg_buffer, np.array(new_eeg_sample), notch=True,
        #         filter_state=filter_state)
        # # eeg_buffer, filter_state = update_buffer(eeg_buffer, np.array(new_eeg_sample))


        # """ 3.2 COMPUTE FEATURES AND CLASSIFY """
        # # Get newest <epoch_size> samples from the buffer
        # data_epoch = get_last_data(eeg_buffer,
        #                                 epoch_size)

        # # Compute features
        # feat_vector = compute_feature_vector(np.array(new_eeg_sample), NOMINAL_SAMPLING_RATE)
        # y_hat = test_classifier(classifier,
        #                                 feat_vector.reshape(1, -1), mu_ft,
        #                                 std_ft)
        # print(f"prediction: {y_hat}")

        # decision_buffer, _ = update_buffer(decision_buffer,
        #                                         np.reshape(y_hat, (-1, 1)))

        # """ 3.3 VISUALIZE THE DECISIONS """
        # plotter_decision.update_plot(decision_buffer)
        # plt.pause(0.00001)
except KeyboardInterrupt:

    print('Closed!')