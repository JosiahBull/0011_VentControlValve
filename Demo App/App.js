import React, { Component } from 'react';
import { StyleSheet, Button, View, SafeAreaView, Text, Alert, TouchableOpacity } from 'react-native';

const styles = StyleSheet.create({
	container: {
	  flex: 1,
	  justifyContent: 'center',
	  marginHorizontal: 16,
	},
	title: {
	  textAlign: 'center',
	  marginVertical: 8,
	},
	fixToText: {
	  flexDirection: 'row',
	  justifyContent: 'space-between',
	},
	separator: {
	  marginVertical: 8,
	  borderBottomColor: '#737373',
	  borderBottomWidth: StyleSheet.hairlineWidth,
	},
});
const Separator = () => (
	<View style={styles.separator} />
);

class Dashboard extends Component {
	intervalID;
	state = {
		data: [],
	}
	componentDidMount() {
		this.getStatus();
	}
	componentWillUnmount() {
		clearTimeout(this.intervalID);
	}
	getStatus = async () => {
		try {
			const requestOptions = {
				method: 'GET'
			}
			const response = await fetch('http://192.168.0.130', requestOptions);
			const data = await response.json();
			this.setState({data: {
				shutterOpen: (data.shutterOpen === '1'), 
				errorStatus: (data.errorStatus === '1'),
				connected: true
			}})
		} catch (err) {
			//Runs when unable to contact vent for refresh.
			this.setState({data: {
				shutterOpen: this.state.data.shutterOpen,
				errorStatus: this.state.data.errorStatus,
				connected: false
			}})
		}
		this.intervalID = setTimeout(this.getStatus.bind(this), 5000);
	}
	openVent = async () => {
		if (!this.state.data.connected) return; //If not connected to vent then don't attempt to run.
		this.state.data.shutterOpen = true;
		const requestOptions = {
			method: 'POST',
			headers: { 'Content-Type': 'application/json' },
			body: JSON.stringify({ title: 'Beep' })
		};
		const response = await fetch('http://192.168.0.130/open', requestOptions);
		const data = await response.json();
	};
	closeVent = async () => {
		if (!this.state.data.connected) return; //If not connected to vent then don't attempt to run.
		this.state.data.shutterOpen = false;
		const requestOptions = {
			method: 'POST',
			headers: { 'Content-Type': 'application/json' },
			body: JSON.stringify({title: 'Beep'})
		};
		const response = await fetch('http://192.168.0.130/close', requestOptions);
		const data = await response.json();
	}
	resetVent = async () => {
		if (!this.state.data.connected) return; //If not connected to vent then don't attempt to run.
		const requestOptions = {
			method: 'POST',
			headers: { 'Content-Type': 'application/json' },
			body: JSON.stringify({ title: '' })
		}
		const response = await fetch('http://192.168.0.130/reset', requestOptions);
		const data = await response.json();
	}
	currentStatus = () => {
		return <Text style={styles.title}>The vent is currently {(this.state.data.shutterOpen) ? 'open' : 'closed'}.</Text>
	}
	errorDisplay = () => {
		if (this.state.data.errorStatus) {
			<Separator />
			return <Text style={styles.title}>The vent is currently suffering from an error!</Text>;
		}
		return null;
	}
	connectedDisplay = () => {
		if (!this.state.data.connected) {
			<Separator />
			return <Text style={styles.title}>Unable to connect to vent, check your connection.</Text>;
		}
		return null;
	}
	render() {
		return (
			<SafeAreaView style={styles.container}>
				<View>
					<this.currentStatus />
				</View>
				<Separator />
				<View>
					<Text style={styles.title}>Select an action for the vent to perform.</Text>
					<View style={styles.fixToText}>
					<TouchableOpacity disabled={this.state.data.connected}>
						<Button
							title="Open Vent"
							onPress={this.openVent}
					/>
					</TouchableOpacity>
					<TouchableOpacity disabled={this.state.data.errorStatus && this.state.data.connected}>
						<Button
							title="Reset Vent"
							onPress={this.resetVent}
						/>
					</TouchableOpacity>
					<TouchableOpacity disabled={this.state.data.connected}>
						<Button
							title="Close Vent"
							onPress={this.closeVent}
						/>
					</TouchableOpacity>
					</View>
					<this.errorDisplay />
					<this.connectedDisplay />
				</View>
			</SafeAreaView>
		);
	}
}

export default Dashboard;